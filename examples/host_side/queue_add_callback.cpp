#include <iostream>
#include <queue>

// Includes common necessary includes for development using depthai library
#include "depthai/depthai.hpp"

struct callbackType {
    std::string name;
    cv::Mat frame;
};

int main() {
    // Create pipeline
    dai::Pipeline pipeline;

    // Add all three cameras
    auto camRgb = pipeline.create<dai::node::ColorCamera>();
    auto left = pipeline.create<dai::node::MonoCamera>();
    auto right = pipeline.create<dai::node::MonoCamera>();

    // Create XLink output
    auto xout = pipeline.create<dai::node::XLinkOut>();
    xout->setStreamName("frames");

    // Properties
    camRgb->setPreviewSize(300, 300);
    left->setBoardSocket(dai::CameraBoardSocket::LEFT);
    left->setResolution(dai::MonoCameraProperties::SensorResolution::THE_400_P);
    right->setBoardSocket(dai::CameraBoardSocket::RIGHT);
    right->setResolution(dai::MonoCameraProperties::SensorResolution::THE_400_P);

    // Stream all the camera streams through the same XLink node
    camRgb->preview.link(xout->input);
    left->out.link(xout->input);
    right->out.link(xout->input);

    auto script = pipeline.create<dai::node::Script>();
    script->setScript(R"(
        import json
        import time

        # Predefined buffer variables used for sending result to host
        buf1 = NNData(169)

        while True:
            buf1.setLayer("type", [float(0.77)])
            buf1.setLayer("lm_score", [float(0.55)])
            node.io['out'].send(buf1)
            time.sleep(1)
    )");
    script->outputs["out"].link(xout->input);

    auto queue = std::queue<callbackType>();
    std::mutex queueMtx;

    // Connect to device and start pipeline
    dai::Device device(pipeline);

    auto newFrame = [&queueMtx, &queue](std::shared_ptr<dai::ADatatype> callback) {
        if(dynamic_cast<dai::ImgFrame*>(callback.get()) != nullptr) {
            std::unique_lock<std::mutex> lock(queueMtx);
            callbackType cb;
            dai::ImgFrame* imgFrame = static_cast<dai::ImgFrame*>(callback.get());
            auto num = imgFrame->getInstanceNum();
            cb.name = num == 0 ? "color" : (num == 1 ? "left" : "right");
            cb.frame = imgFrame->getCvFrame();
            queue.push(cb);
        } else if(dynamic_cast<dai::NNData*>(callback.get()) != nullptr) {
            auto _frameLocal = static_cast<dai::NNData*>(callback.get());
            std::cout << "getAllLayers.size() = " << _frameLocal->getAllLayers().size() << "\n";
            const auto lmScore = _frameLocal->getLayerFp16("lm_score")[0];
            std::cout << "lmScore" << lmScore << "\n";
        }
    };

    // Add callback to the output queue "frames" for all newly arrived frames (color, left, right)
    device.getOutputQueue("frames", 4, false)->addCallback(newFrame);

    while(true) {
        callbackType data;
        {
            std::unique_lock<std::mutex> lock(queueMtx);
            if(!queue.empty()) {
                data = queue.front();
                queue.pop();
            }
        }

        if(!data.frame.empty()) {
            cv::imshow(data.name.c_str(), data.frame);
        }

        int key = cv::waitKey(1);
        if(key == 'q' || key == 'Q') {
            return 0;
        }
    }
    return 0;
}

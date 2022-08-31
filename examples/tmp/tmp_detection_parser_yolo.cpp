
#include <iostream>

#include "utility.hpp"

// Includes common necessary includes for development using depthai library
#include "depthai/depthai.hpp"

static constexpr auto MODEL_IN_WIDTH = 416;
static constexpr auto MODEL_IN_HEIGHT = 416;

static const std::vector<std::string> labelMap = {
    "person",        "bicycle",      "car",           "motorbike",     "aeroplane",   "bus",         "train",       "truck",        "boat",
    "traffic light", "fire hydrant", "stop sign",     "parking meter", "bench",       "bird",        "cat",         "dog",          "horse",
    "sheep",         "cow",          "elephant",      "bear",          "zebra",       "giraffe",     "backpack",    "umbrella",     "handbag",
    "tie",           "suitcase",     "frisbee",       "skis",          "snowboard",   "sports ball", "kite",        "baseball bat", "baseball glove",
    "skateboard",    "surfboard",    "tennis racket", "bottle",        "wine glass",  "cup",         "fork",        "knife",        "spoon",
    "bowl",          "banana",       "apple",         "sandwich",      "orange",      "broccoli",    "carrot",      "hot dog",      "pizza",
    "donut",         "cake",         "chair",         "sofa",          "pottedplant", "bed",         "diningtable", "toilet",       "tvmonitor",
    "laptop",        "mouse",        "remote",        "keyboard",      "cell phone",  "microwave",   "oven",        "toaster",      "sink",
    "refrigerator",  "book",         "clock",         "vase",          "scissors",    "teddy bear",  "hair drier",  "toothbrush"};

int main(int argc, char** argv) {
    int camId = 0;
    if(argc > 1) {
        camId = std::stoi(argv[1]);
    }

    // Create pipeline
    dai::Pipeline pipeline;

    // Define sources and outputs
    auto nn = pipeline.create<dai::node::YoloDetectionNetwork>();
    auto camRgb = pipeline.create<dai::node::ColorCamera>();
    auto xoutRgb = pipeline.create<dai::node::XLinkOut>();
    auto xoutDetections = pipeline.create<dai::node::XLinkOut>();

    camRgb->setPreviewSize(MODEL_IN_WIDTH, MODEL_IN_HEIGHT);
    camRgb->setColorOrder(dai::ColorCameraProperties::ColorOrder::BGR);

    camRgb->setInterleaved(false);

    nn->setBlobPath(MODEL_BLOB_PATH);
    nn->setNumInferenceThreads(2);

    xoutRgb->setStreamName("rgb");
    xoutDetections->setStreamName("nn_out");;

    // Linking
    camRgb->preview.link(nn->input);
    camRgb->preview.link(xoutRgb->input);
    nn->out.link(xoutDetections->input);

    // Detection network settings
    nn->setConfidenceThreshold(0.5f);
    nn->setNumClasses(80);
    nn->setCoordinateSize(4);
    nn->setAnchors({10, 14, 23, 27, 37, 58, 81, 82, 135, 169, 344, 319});
    nn->setAnchorMasks({{"side26", {1, 2, 3}}, {"side13", {3, 4, 5}}});
    nn->setIouThreshold(0.5f);

    // Connect to device and start pipeline
    dai::Device device(pipeline);

    auto detections = device.getOutputQueue("nn_out");
    auto rgbOutQ = device.getOutputQueue("rgb", 4, false);

    // Add bounding boxes and text to the frame and show it to the user
    auto displayFrame = [](std::string name, cv::Mat frame, std::vector<dai::ImgDetection>& detections) {
        auto color = cv::Scalar(255, 0, 0);
        // nn data, being the bounding box locations, are in <0..1> range - they need to be normalized with frame width/height
        for(auto& detection : detections) {
            int x1 = detection.xmin * frame.cols;
            int y1 = detection.ymin * frame.rows;
            int x2 = detection.xmax * frame.cols;
            int y2 = detection.ymax * frame.rows;

            uint32_t labelIndex = detection.label;
            std::string labelStr = std::to_string(labelIndex);
            if(labelIndex < labelMap.size()) {
                labelStr = labelMap[labelIndex];
            }
            cv::putText(frame, labelStr, cv::Point(x1 + 10, y1 + 20), cv::FONT_HERSHEY_TRIPLEX, 0.5, 255);
            std::stringstream confStr;
            confStr << std::fixed << std::setprecision(2) << detection.confidence * 100;
            cv::putText(frame, confStr.str(), cv::Point(x1 + 10, y1 + 40), cv::FONT_HERSHEY_TRIPLEX, 0.5, 255);
            cv::rectangle(frame, cv::Rect(cv::Point(x1, y1), cv::Point(x2, y2)), color, cv::FONT_HERSHEY_SIMPLEX);
        }
        // Show the frame
        cv::imshow(name, frame);
    };

    std::vector<dai::ImgDetection> vDetections;

    using namespace std::chrono;
    auto startTime = steady_clock::now();
    int counter = 0;
    float fps = 0;

    while(true) {
        std::shared_ptr<dai::ImgFrame> inRgb;
        inRgb = rgbOutQ->get<dai::ImgFrame>();
        auto frame = inRgb->getCvFrame();

        counter++;
        auto currentTime = steady_clock::now();
        auto elapsed = duration_cast<duration<float>>(currentTime - startTime);
        if(elapsed > seconds(1)) {
            fps = counter / elapsed.count();
            counter = 0;
            startTime = currentTime;
        }

        auto inDet = detections->tryGet<dai::ImgDetections>();
        if(inDet) {
            vDetections = inDet->detections;
            std::stringstream fpsStr;
            fpsStr << "NN fps: " << std::fixed << std::setprecision(2) << fps;
            cv::putText(frame, fpsStr.str(), cv::Point(2, MODEL_IN_HEIGHT - 4), cv::FONT_HERSHEY_TRIPLEX, 0.4, cv::Scalar(255, 255, 255));
        }
        displayFrame("nn_out", frame, vDetections);
        int key = cv::waitKey(1);
        if(key == 'q') {
            return 0;
        }
    }
}

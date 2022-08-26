#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

// Include depthai library
#include <depthai/depthai.hpp>

using namespace std::chrono;
using namespace std::chrono_literals;

auto TIMEOUT = 5s;
constexpr auto MIN_NUM_FRAMES = 3;
constexpr auto WIDTH = 1000;
constexpr auto HEIGHT = 1000;

static void testXlinkRoundTripWithImageManip(int numOfFrames, bool linkImageManip) {
    std::vector<std::uint8_t> data(WIDTH * HEIGHT * 3);

    dai::Pipeline p;
    auto xIn = p.create<dai::node::XLinkIn>();
    xIn->setStreamName("to_device");
    xIn->setMaxDataSize(data.size());
    xIn->setNumFrames(numOfFrames);

    auto imageManip = p.create<dai::node::ImageManip>();
    imageManip->initialConfig.setFrameType(dai::ImgFrame::Type::BGR888p);
    imageManip->initialConfig.setResize(300, 300);
    imageManip->inputImage.setBlocking(false);

    auto xOut = p.create<dai::node::XLinkOut>();
    xOut->setStreamName("to_host");

    // Linking
    if(linkImageManip){
        xIn->out.link(imageManip->inputImage);
        imageManip->out.link(xOut->input);
    } else {
        xIn->out.link(xOut->input);
    }

    dai::Device device(p);

    auto outQ = device.getOutputQueue("to_host");
    auto inQ = device.getInputQueue("to_device");
    inQ->setBlocking(false);

    dai::ImgFrame imgFrame;

    imgFrame.setSequenceNum(123);
    imgFrame.setWidth(WIDTH);
    imgFrame.setHeight(HEIGHT);

    imgFrame.setData(data);
    imgFrame.setType(dai::RawImgFrame::Type::BGR888p);

    auto t1 = steady_clock::now();
    int receivedImages = 0;
    do {
        // Send the frame
        inQ->send(imgFrame);
        auto retFrm = outQ->tryGet();
        if(retFrm) {
            receivedImages++;
            REQUIRE(imgFrame.getSequenceNum() == 123);
        }
        if(receivedImages >= MIN_NUM_FRAMES) {
            return;
        }
    } while(steady_clock::now() - t1 < TIMEOUT);
    // Timeout
    FAIL("Timeout receiving back the sent message");
}

TEST_CASE("Test XLinkIn->XLinkOut with pool size 1") {
    testXlinkRoundTripWithImageManip(1, false);
}

TEST_CASE("Test XLinkIn->ImageManip->XLinkOut with pool size 1") {
    testXlinkRoundTripWithImageManip(1, true);
}

TEST_CASE("Test XLinkIn->ImageManip->XLinkOut with pool size 2") {
    testXlinkRoundTripWithImageManip(2, true);
}

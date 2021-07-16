// C++ GUI test program to read an image and convert it to gray with OpenCV 4
// You can compile the program with:
//     g++ gui_cpp_test.cpp -o gui_cpp_test `pkg-config --cflags --libs opencv`
// Be sure that you have an image file named "lake.jpg" in the work folder and run the code with:
//     ./gui_cpp_test

#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/core/utility.hpp>

#include <time.h>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <errno.h>
#include <math.h>
#include <memory>
#include <thread>
#include <vector>
#include <utility>

#include "Utils.h"

#include "DetectFacesStage.h"

#include "ThreadPool.h"
#include "SharedQueue.h"

#include "Scheduler.h"

const uint32_t IN_BUFFER_SIZE = 16;
const uint32_t MAX_THREAD = std::thread::hardware_concurrency();

void printHelp () {
    std::cout << "raspidms [0|PATH_TO_VIDEO.mp4] [haar|resnetCaffe|yoloResnet18|yoloEffnetb0]" << std::endl;
}

typedef std::function<std::pair<std::vector<cv::Rect>, double>(cv::Mat frame)> DetectFaceFunc;

int main(int argc, char**argv) {
    if (argc < 3) {
        printHelp();
        return 0;
    }

    const bool multithread = true;
    const std::string video_path(argv[1]);
    const std::string firstDetector = argc > 2 ? argv[2] : "";
    const std::string secondDetector = argc > 3 ? argv[3] : "";

    cv::VideoCapture cap;

    if (video_path == "0") {
        cap.open(0);
    } else {
        cap.open(video_path);
    }

    if (! cap.isOpened()) {
        std::cerr << "Can't open file " << video_path << std::endl;
    }

    std::cout << "Start grabbing" << std::endl;

    ThreadPool pool(MAX_THREAD);
    std::shared_ptr<SharedQueue<cv::Mat>> inFrames(new SharedQueue<cv::Mat>());
    std::shared_ptr<SharedQueue<DetectedFacesResult>> outRects(new SharedQueue<DetectedFacesResult>());

    // Responsible of detecting faces, needs in frames, and ouputs out rectangles
    DetectFacesStage detectFacesStage(inFrames, outRects);

    // Used to periodically push the detectFacesStage in the threadPool, and also schedule the various detectors
    // into the detectFacesStage
    Scheduler scheduler;
    scheduler.addFunc([&](){ if (pool.queue_size() < MAX_THREAD) pool.push(std::ref(detectFacesStage));
        }, 0.05, 0.05);
    long firstId = scheduler.addFunc([&](){ detectFacesStage.schedNextDetector(firstDetector); }, 0.2, 0.1);
    long secondId = scheduler.addFunc([&](){ detectFacesStage.schedNextDetector(secondDetector); }, 0.5, 0.1);

    cv::namedWindow("Head", cv::WINDOW_AUTOSIZE);
    cv::Mat frame;
    DetectedFacesResult rectsToDraw;
    for(;;)
    {
        if (cv::pollKey() >= 0) {
            pool.stop();
            break;
        }

        // lose frames if too slow
        if (inFrames->size() >= IN_BUFFER_SIZE) {
            inFrames->pop_front_no_wait();
        }

        if (inFrames->size() < IN_BUFFER_SIZE) {
            // wait for a new frame from camera and store it into 'frame'
            cap.read(frame);
            // check if we succeeded
            if (frame.empty()) {
                std::cerr << "ERROR! blank frame grabbed\n";
                break;
            }

            //TODO consider no copy
            inFrames->push_back(frame);
        }

        if (multithread) {
            scheduler.schedule();
        } else {
            detectFacesStage(0);
        }

        DetectedFacesResult rects;
        if (outRects->pop_front_no_wait(rects) && rects.first.size() > 0) {
            rectsToDraw = rects;
        }

        for(const auto & rect : rectsToDraw.first) {
            rectangle(frame, cv::Point(rect.x, rect.y),
                      cv::Point(rect.x + rect.width, rect.y + rect.height),
                      cv::Scalar(255, 0, 0),
                      3, 8, 0);
        }

        cv::imshow("Head", frame);
    }
    return 0;
}

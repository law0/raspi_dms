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

#include "TimeMark.hpp"

#include "DetectFacesStage.h"

#include "ThreadPool.h"
#include "SharedQueue.h"

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

    std::shared_ptr<SharedQueue<cv::Mat>> inFrames(new SharedQueue<cv::Mat>());
    std::shared_ptr<SharedQueue<DetectedFacesResult>> outRects(new SharedQueue<DetectedFacesResult>());
    DetectFacesStage detectFacesStage(inFrames, outRects);
    ThreadPool pool(MAX_THREAD);

    clock_t lastTime20 = clock();
    clock_t lastTime50 = lastTime20;
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
            clock_t currTime = clock();
            clock_t elapsed20 = currTime - lastTime20;
            clock_t elapsed50 = currTime - lastTime50;

            if (elapsed20 / CLOCKS_PER_SEC > 0.20) {
                lastTime20 = currTime;
                detectFacesStage.schedNextDetector(firstDetector);
            }

            if (elapsed50 / CLOCKS_PER_SEC > 0.50) {
                lastTime50 = currTime;
                detectFacesStage.schedNextDetector(secondDetector);
            }

            //thread pool takes frame from inFrames and output to outFrames
            //there is actually a queue in pool, that take what you push
            //in order. But I don't want to grow the queue infintely
            if (pool.queue_size() < MAX_THREAD) {
                pool.push(std::ref(detectFacesStage));
            }
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

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

#include "DetectFaces/DetectFacesStage.h"
#include "FaceFeatures/FaceFeaturesStage.h"

//#include "FaceFeaturesDlib.h"


#include "ThreadPool.h"
#include "SharedQueue.h"

#include "Scheduler.h"

const uint32_t IN_BUFFER_SIZE = 4;
const uint32_t MAX_THREAD = std::thread::hardware_concurrency();

void printHelp () {
    std::cout << "raspidms [0|PATH_TO_VIDEO.mp4] [haar|tflite|resnetCaffe|yoloResnet18|yoloEffnetb0]" << std::endl;
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

    // In queue of frames
    std::shared_ptr<SharedQueue<cv::Mat>> inputFrameQueue(new SharedQueue<cv::Mat>());

    // Out queue of rects to draw (and region of interests for feature detection)
    std::shared_ptr<SharedQueue<DetectedFacesResult>> rectsQueue(new SharedQueue<DetectedFacesResult>());
    std::shared_ptr<SharedQueue<DetectedFacesResult>> meanRectsQueue(new SharedQueue<DetectedFacesResult>());

    // Face feature to be drawn
    std::shared_ptr<SharedQueue<FaceFeaturesResult>> faceFeaturesQueue(new SharedQueue<FaceFeaturesResult>());

    // Responsible of detecting faces, needs in frames, and ouputs out rectangles
    DetectFacesStage detectFacesStage(firstDetector, inputFrameQueue, rectsQueue);


    meanRectsQueue->push_back({{cv::Rect()}, 0.});

    // This is temporary
    // calculate the exp moving average of rects in rectsQueue and put it
    // in meanRectsQueue (which faceFeaturesStage will pull from)
    // This avoid jumps in region of interest
    // stuck to only one rects though
    auto rectsMeanCalculator = [&](int) {
            DetectedFacesResult dfr;
            DetectedFacesResult& mdfr = meanRectsQueue->front_wait();
            cv::Rect& rect = mdfr.first[0];
            double alpha = 0.7;
            if (rectsQueue->pop_front_no_wait(dfr) && dfr.first.size() > 0) {
                rect.x = alpha * dfr.first[0].x + (1. - alpha) * rect.x;
                rect.y = alpha * dfr.first[0].y + (1. - alpha) * rect.y;
                rect.width = alpha * dfr.first[0].width + (1. - alpha) * rect.width;
                rect.height = alpha * dfr.first[0].height + (1. - alpha) * rect.height;
            }
    };


    // Responsible for detecting face feature (landmarks)
    FaceFeaturesStage faceFeaturesStage("dlib_68", inputFrameQueue, meanRectsQueue, faceFeaturesQueue);

    Scheduler scheduler;

    scheduler.addFunc(std::ref(detectFacesStage));
    scheduler.addFunc(rectsMeanCalculator);
    scheduler.addFunc(std::ref(faceFeaturesStage));
    //scheduler.addFunc([](int i){ std::cout << "----------> test: " << i << " <------ " << std::endl; }, 1);

    DetectedFacesResult rects;
    FaceFeaturesResult faceFeatures;

    cv::namedWindow("Head", cv::WINDOW_AUTOSIZE);
    cv::Mat frame;
    for(;;)
    {
        if (cv::pollKey() >= 0) {
            break;
        }

        // lose frames if too slow
        if (inputFrameQueue->size() >= IN_BUFFER_SIZE)
            inputFrameQueue->pop_front_no_wait();

        if (inputFrameQueue->size() < IN_BUFFER_SIZE) {
            // wait for a new frame from camera and store it into 'frame'
            cap.read(frame);
            // check if we succeeded
            if (frame.empty()) {
                std::cerr << "ERROR! blank frame grabbed\n";
                break;
            }

            //TODO consider no copy
            inputFrameQueue->push_back(frame);
        }

        if (multithread) {
            scheduler.schedule();
        } else {
            detectFacesStage(0);
            faceFeaturesStage(0);
        }

        DetectedFacesResult dfr;
        if (meanRectsQueue->front_no_wait(dfr) /*&& dfr.first.size() > 0*/)
            rects = dfr;

        if (rects.first.size() > 0) {
            for(const auto & rect : rects.first) {
                rectangle(frame, cv::Point(rect.x, rect.y),
                          cv::Point(rect.x + rect.width, rect.y + rect.height),
                          cv::Scalar(255, 0, 0),
                          3, 8, 0);
            }
        }

        FaceFeaturesResult ffr;
        if (faceFeaturesQueue->front_no_wait(ffr)) {
            if (ffr.first.size() > 0) {
                faceFeatures = ffr;
            } else {
                std::cout << "Empty face features ! This should not happen !" << std::endl;
            }
        }

        if (faceFeatures.first.size() > 0) {
            for(const auto & vecpoints : faceFeatures.first) {
                for(const auto & point : vecpoints) {
                    circle(frame, point, 4, cv::Scalar(0, 0, 255), cv::FILLED, cv::LINE_8);
                }
            }
        }

        cv::imshow("Head", frame);
    }
    return 0;
}

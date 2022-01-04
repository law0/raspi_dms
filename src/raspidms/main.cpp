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
    std::shared_ptr<SharedQueue<PointsList>> rectsQueue(new SharedQueue<PointsList>());
    std::shared_ptr<SharedQueue<PointsList>> meanRectsQueue(new SharedQueue<PointsList>());

    // Face feature to be drawn
    std::shared_ptr<SharedQueue<PointsList>> faceFeaturesQueue(new SharedQueue<PointsList>());

    // Responsible of detecting faces, needs in frames, and ouputs out rectangles
    DetectFacesStage detectFacesStage(firstDetector, inputFrameQueue, rectsQueue);


    meanRectsQueue->push_back({{cv::Point2f(), cv::Point2f()}});

    // This is temporary
    // calculate the exp moving average of rects in rectsQueue and put it
    // in meanRectsQueue (which faceFeaturesStage will pull from)
    // This avoid jumps in region of interest
    // stuck to only one rects though
    auto rectsMeanCalculator = [&](int) {
            PointsList dfr;
            PointsList& mdfr = meanRectsQueue->front_wait();
            std::vector<cv::Point2f>& rect = mdfr[0];
            double alpha = 0.7;
            if (rectsQueue->pop_front_no_wait(dfr) && dfr.size() > 0) {
                rect[0].x = alpha * dfr[0][0].x + (1. - alpha) * rect[0].x;
                rect[0].y = alpha * dfr[0][0].y + (1. - alpha) * rect[0].y;
                rect[1].x = alpha * dfr[0][1].x + (1. - alpha) * rect[1].x;
                rect[1].y = alpha * dfr[0][1].y + (1. - alpha) * rect[1].y;
            }
    };


    // Responsible for detecting face feature (landmarks)
    FaceFeaturesStage faceFeaturesStage("dlib_68", inputFrameQueue, meanRectsQueue, faceFeaturesQueue);

    Scheduler scheduler;

    scheduler.addFunc(std::ref(detectFacesStage));
    scheduler.addFunc(rectsMeanCalculator);
    scheduler.addFunc(std::ref(faceFeaturesStage));

    PointsList rects;
    PointsList face_features;

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

        PointsList bounding_boxes;
        if (meanRectsQueue->front_no_wait(bounding_boxes))
            rects = bounding_boxes;

        for(const auto & points : rects) {
            if (points.size() > 1)
                rectangle(frame, cv::Point(points[0].x, points[0].y),
                        cv::Point(points[1].x, points[1].y),
                        cv::Scalar(255, 0, 0),
                        3, 8, 0);
        }


        PointsList features;
        if (faceFeaturesQueue->front_no_wait(features)) {
            if (features.size() > 0) {
                face_features = features;
            } else {
                std::cout << "Empty face features ! This should not happen !" << std::endl;
            }
        }

        if (face_features.size() > 0) {
            for(const auto & vecpoints : face_features) {
                for(const auto & point : vecpoints) {
                    circle(frame, point, 4, cv::Scalar(0, 0, 255), cv::FILLED, cv::LINE_8);
                }
            }
        }

        cv::imshow("Head", frame);
    }
    return 0;
}

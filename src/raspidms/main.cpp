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

const uint32_t IN_BUFFER_SIZE = 8;

void printHelp () {
    std::cout << "raspidms [0|PATH_TO_VIDEO.mp4] [haar|tflite|resnetCaffe|yoloResnet18|yoloEffnetb0]" << std::endl;
}

typedef std::function<std::pair<std::vector<cv::Rect>, double>(cv::Mat frame)> DetectFaceFunc;

typedef std::chrono::duration<double, std::ratio<1, 120>> FrameDuration;

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

    // Face feature to be drawn
    std::shared_ptr<SharedQueue<PointsList>> faceFeaturesQueue(new SharedQueue<PointsList>());

    // Responsible of detecting faces, needs in frames, and ouputs out rectangles
    DetectFacesStage detectFacesStage(firstDetector, inputFrameQueue, rectsQueue);

    // Responsible for detecting face feature (landmarks)
    FaceFeaturesStage faceFeaturesStage("dlib_68", inputFrameQueue, rectsQueue, faceFeaturesQueue);

    Scheduler scheduler;

    scheduler.addFunc([&](int threadId) {
        detectFacesStage(threadId);
        faceFeaturesStage(threadId);
    });

    PointsList rects;
    PointsList face_features;

    cv::namedWindow("Head", cv::WINDOW_AUTOSIZE);
    cv::Mat frame;
    auto last = std::chrono::high_resolution_clock::now();
    auto now = last;
    for(;;)
    {
        if (cv::pollKey() >= 0) {
            break;
        }

        // lose frames if too slow
        if (inputFrameQueue->size() > IN_BUFFER_SIZE)
            inputFrameQueue->pop_front_no_wait();

        while (inputFrameQueue->size() < IN_BUFFER_SIZE) {
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

        //120 fps max, otherwise we may starve of frames
        FrameDuration duration = now - last;
        if (duration.count() < 1.0)
            std::this_thread::sleep_for(FrameDuration(1.0 - duration.count()));

        last = now;
        now = std::chrono::high_resolution_clock::now();

        if (multithread) {
            scheduler.schedule();
        } else {
            detectFacesStage(0);
            faceFeaturesStage(0);
        }

        PointsList bounding_boxes;
        if (rectsQueue->front_no_wait(bounding_boxes) && bounding_boxes.size() > 0) {
            rects = bounding_boxes;
        }

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

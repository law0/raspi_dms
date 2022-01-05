#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/core/utility.hpp>

#include <getopt.h>
#include <iostream>
#include <errno.h>
#include <math.h>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <utility>
#include <vector>

#include "Utils.h"

#include "DetectFaces/DetectFacesStage.h"
#include "FaceFeatures/FaceFeaturesStage.h"

#include "ThreadPool.h"
#include "SharedQueue.h"

#include "Scheduler.h"

const uint32_t MAX_IN_BUFFER_SIZE = 8;

void printHelp () {
    std::cout << "USAGE: " << std::endl
    << "raspidms OPTIONS" << std::endl
    << "with OPTIONS being ([] are optionals, rest is mandatory. A|B means A or B) :" << std::endl
    << "    -d|--face-detector haar|tflite|resnetCaffe|yoloResnet18|yoloEffnetb0" << std::endl
    << "    -m|--face-mesh dlib_68" << std::endl
    << "    [-j|--multithread]" << std::endl
    << "    [-h|--help]" << std::endl
    << "    0|PATH_TO_VIDEO.mp4" << std::endl;
}

typedef std::function<std::pair<std::vector<cv::Rect>, double>(cv::Mat frame)> DetectFaceFunc;

struct Args {
    std::string video_path;
    std::string face_detector_model;
    std::string face_mesh_model;
    bool multithread;
};

struct Args parseArgs(int argc, char** argv) {

    struct Args args;
    args.multithread = false;

    //Specifying the expected options
    //The two options l and b expect numbers as argument
    static struct option long_options[] = {
    {"face-detector",  required_argument,  0,  'd' },
    {"face-mesh",      required_argument,  0,  'm' },
    {"multithread",    no_argument,        0,  'j' },
    {"help",           no_argument,        0,  'h' },
    {0, 0, 0, 0},
    };

    char opt = 0;
    int long_index = 0;
    while ((opt = getopt_long(argc, argv, "d:m:jh",
                              long_options, &long_index )) != -1) {
        switch (opt) {
            case 'd' :
                args.face_detector_model = std::string(optarg);
                break;
            case 'm' :
                args.face_mesh_model = std::string(optarg);
                break;
            case 'j':
                args.multithread = true;
                break;
            case 'h':
                printHelp();
                exit(EXIT_SUCCESS);
            default:
                printHelp();
                exit(EXIT_FAILURE);
        }
    }

    if (optind < argc && argc - optind == 1) {
        args.video_path = std::string(argv[argc - 1]);
    } else {
        printHelp();
        exit(EXIT_FAILURE);
    }

    return args;
}

int main(int argc, char**argv) {
    const struct Args args = parseArgs(argc, argv);

    cv::VideoCapture cap;

    if (!args.video_path.empty() && std::all_of(args.video_path.begin(), args.video_path.end(), ::isdigit)) {
        cap.open(stoi(args.video_path));
    } else {
        cap.open(args.video_path);
    }

    if (! cap.isOpened()) {
        std::cerr << "Can't open file " << args.video_path << std::endl;
    }

    std::cout << "Start grabbing" << std::endl;

    // In queue of frames
    std::shared_ptr<SharedQueue<cv::Mat>> inputFrameQueue(new SharedQueue<cv::Mat>());

    // Out queue of rects to draw (and region of interests for feature detection)
    std::shared_ptr<SharedQueue<PointsList>> rectsQueue(new SharedQueue<PointsList>());

    // Face feature to be drawn
    std::shared_ptr<SharedQueue<PointsList>> faceFeaturesQueue(new SharedQueue<PointsList>());

    // Responsible of detecting faces, needs in frames, and ouputs out rectangles
    DetectFacesStage detectFacesStage(args.face_detector_model, inputFrameQueue, rectsQueue);

    // Responsible for detecting face feature (landmarks)
    FaceFeaturesStage faceFeaturesStage(args.face_mesh_model, inputFrameQueue, rectsQueue, faceFeaturesQueue);

    Scheduler scheduler;

    scheduler.addFunc([&](int threadId) {
        detectFacesStage(threadId);
        faceFeaturesStage(threadId);
    });

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
        while (inputFrameQueue->size() > MAX_IN_BUFFER_SIZE)
            inputFrameQueue->pop_front_no_wait();

        // wait for a new frame from camera and store it into 'frame'
        cap.read(frame);
        // check if we succeeded
        if (frame.empty()) {
            std::cerr << "ERROR! blank frame grabbed\n";
            break;
        }

        //TODO consider no copy
        inputFrameQueue->push_back(frame);

        if (args.multithread) {
            scheduler.schedule();
        } else {
            detectFacesStage(0);
            faceFeaturesStage(0);
        }


        // emptying down to most recent bounding box
        PointsList bounding_boxes;
        while (rectsQueue->size() > 1) {
            rectsQueue->pop_front_no_wait(bounding_boxes);
        }

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

        // emptying down to most recent face features
        PointsList features;
        while (faceFeaturesQueue->size() > 1) {
            faceFeaturesQueue->pop_front_no_wait(features);
        }

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

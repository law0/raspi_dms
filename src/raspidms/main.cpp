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

#include "DetectFacesEmpty.h"
#include "DetectFacesHaar.h"
#include "DetectFacesMyYolo.h"
#include "DetectFacesResnetCaffe.h"
#include "DetectFacesHoG.h"

#include "ThreadPool.h"
#include "SharedQueue.h"

const std::string HAAR_CASCADE_PATH = "haarcascades/haarcascade_frontalface_default.xml";
const std::string RESNET_CAFFE_PROTO_TXT_PATH = "../res/Resnet_SSD_deploy.prototxt";
const std::string RESNET_CAFFE_MODEL_PATH = "../res/Res10_300x300_SSD_iter_140000.caffemodel";
const std::string MY_YOLO_RESNET_18_PATH = "../res/YoloResnet18.onnx";
const std::string MY_YOLO_EFFNET_B0_PATH = "../res/YoloEffnetb0.onnx";

const uint32_t IN_BUFFER_SIZE = 16;
const uint32_t OUT_BUFFER_SIZE = 4;
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

    const std::string video_path(argv[1]);
    const std::string detector(argv[2]);
    bool multithread = false;

    if (argc >= 4) {
        multithread = true;
    }

    /**
    /** can also use :
    /** std::unique_ptr<IDetectFaces> detectFaces = std::make_unique<DetectFacesHaar>(haarCascadePath);
     */
    DetectFaceFunc detectFaces = DetectFacesHaar(HAAR_CASCADE_PATH);
    DetectFaceFunc detectFaces2;

    if (detector == "haar")
    {
        detectFaces = DetectFacesHaar(HAAR_CASCADE_PATH);
    }
    else if (detector == "resnetCaffe")
    {
        detectFaces = DetectFacesResnetCaffe(RESNET_CAFFE_PROTO_TXT_PATH, RESNET_CAFFE_MODEL_PATH);
    }
    else if (detector == "haar+resnetCaffe")
    {
        detectFaces = DetectFacesHaar(HAAR_CASCADE_PATH);
        detectFaces2 = DetectFacesResnetCaffe(RESNET_CAFFE_PROTO_TXT_PATH, RESNET_CAFFE_MODEL_PATH);
    }
    else if (detector == "yoloResnet18")
    {
        detectFaces = DetectFacesMyYolo(MY_YOLO_RESNET_18_PATH);
    }
    else if (detector == "yoloEffnetb0")
    {
        detectFaces = DetectFacesMyYolo(MY_YOLO_EFFNET_B0_PATH);
    }
    else if (detector == "hog")
    {
        detectFaces = DetectFacesHoG();
    }
    else if (detector == "haar+yoloEffnetb0")
    {
        detectFaces = DetectFacesHaar(HAAR_CASCADE_PATH);
        detectFaces2 = DetectFacesMyYolo(MY_YOLO_EFFNET_B0_PATH);
    }
    else if (detector == "empty")
    {
        detectFaces = DetectFacesEmpty();
    }

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

    SharedQueue<cv::Mat> inFrames;
    SharedQueue<cv::Mat> outFrames;
    ThreadPool pool(MAX_THREAD);
    std::atomic<long> i(0);

    auto wrapper = [&](DetectFaceFunc& f) -> std::function<void(int)> {
        if(!f)
            return {};

        std::function<void(int)> wrappedFunc = [&](int id) {
            std::cout << "thread id " << id << std::endl;

            //avoid calculus completely if outFrames buffer is full
            if (outFrames.size() >= OUT_BUFFER_SIZE)
                return;

            //TODO consider no copy
            //Don't wait for frame, there should be plenty
            cv::Mat frontFrame;
            if(! inFrames.pop_front_no_wait(frontFrame))
                return;

            // Detect the faces
            std::pair<std::vector<cv::Rect>, double> detectedFaces = f(frontFrame);

            if (i % 10 == 0) {
                std::cout << "time (sec): " << detectedFaces.second << std::endl;
            }
            ++i; //unprotected but who cares

            cv::Scalar blue(255,0,0);
            cv::Scalar red(0,0,255);

            cv::Scalar& color = detectedFaces.second < 0.4 ? blue : red;

            // If possible draw on the latest frame instead on the one
            // we computed
            // It adds fluidity (but ofc adds error)
            cv::Mat potentialFrame;
            if(inFrames.pop_front_no_wait(potentialFrame))
                frontFrame = potentialFrame;

            for(const auto & rect : detectedFaces.first) {
                rectangle(frontFrame, cv::Point(rect.x, rect.y),
                          cv::Point(rect.x + rect.width, rect.y + rect.height),
                          color,
                          3, 8, 0);
            }

            outFrames.push_back(frontFrame);
        };

        return wrappedFunc;
    };

    auto dfw = wrapper(detectFaces);
    auto dfw2 = wrapper(detectFaces2);


    cv::Mat outFrame;

    long counter = 0;
    cv::namedWindow("Head", cv::WINDOW_AUTOSIZE);
    for(;;)
    {
        counter++;

        //Avoid having all thread fire at the same time
        //we want them to round
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (cv::pollKey() >= 0) {
            pool.stop();
            break;
        }

        // lose frames if too slow
        if (inFrames.size() >= IN_BUFFER_SIZE) {
            inFrames.pop_front_no_wait();
        }

        if (inFrames.size() < IN_BUFFER_SIZE) {
            cv::Mat frame;

            // wait for a new frame from camera and store it into 'frame'
            cap.read(frame);
            // check if we succeeded
            if (frame.empty()) {
                std::cerr << "ERROR! blank frame grabbed\n";
                break;
            }

            //TODO consider no copy
            inFrames.push_back(frame);
        }

        // Make sure to fill a whole buffer before starting
        if (counter < IN_BUFFER_SIZE)
            continue;

        if (multithread) {
            //thread pool takes frame from inFrames and output to outFrames
            //there is actually a queue in pool, that take what you push
            //in order. But I don't want to grow the queue infintely
            if (pool.queue_size() < MAX_THREAD) {
                pool.push(dfw);
                if(dfw2)
                    pool.push(dfw2);

            }
        } else {
            dfw(0);
        }

        outFrames.pop_front_wait(outFrame);
        cv::imshow("Head", outFrame);


    }
    return 0;
}

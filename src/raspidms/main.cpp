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
#include <vector>
#include <utility>

#include "TimeMark.hpp"

#include "DetectFacesHaar.h"
#include "DetectFacesMyYolo.h"
#include "DetectFacesResnetCaffe.h"

const std::string haarCascadePath = "haarcascades/haarcascade_frontalface_default.xml";
const std::string resnetCaffeProtoTxtPath = "../res/Resnet_SSD_deploy.prototxt";
const std::string resnetCaffeModelPath = "../res/Res10_300x300_SSD_iter_140000.caffemodel";
const std::string myYoloResnet18Path = "../res/YoloResnet18.onnx";
const std::string myYoloEffnetb0Path = "../res/YoloEffnetb0.onnx";

void printHelp () {
    std::cout << "raspidms [0|PATH_TO_VIDEO.mp4] [haar|resnetCaffe|yoloResnet18|yoloEffnetb0]" << std::endl;
}

int main(int argc, char**argv) {
    if (argc < 3) {
        printHelp();
        return 0;
    }

    const std::string video_path(argv[1]);
    const std::string detector(argv[2]);

    /**
    /** can also use :
    /** std::unique_ptr<IDetectFaces> detectFaces = std::make_unique<DetectFacesHaar>(haarCascadePath);
     */
    std::function<std::pair<std::vector<cv::Rect>, double>(cv::Mat frame)> detectFaces = DetectFacesHaar(haarCascadePath);

    if (detector == "haar") {
        detectFaces = DetectFacesHaar(haarCascadePath);
    } else if (detector == "resnetCaffe") {
        detectFaces = DetectFacesResnetCaffe(resnetCaffeProtoTxtPath, resnetCaffeModelPath);
    } else if (detector == "yoloResnet18") {
        detectFaces = DetectFacesMyYolo(myYoloResnet18Path);
    } else if (detector == "yoloEffnetb0") {
        detectFaces = DetectFacesMyYolo(myYoloEffnetb0Path);
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

    cv::Mat frame;
    cv::namedWindow("Head", cv::WINDOW_AUTOSIZE);
    long i = 0;
    for(;;)
    {
        // wait for a new frame from camera and store it into 'frame'
        cap.read(frame);
        // check if we succeeded
        if (frame.empty()) {
            std::cerr << "ERROR! blank frame grabbed\n";
            break;
        }

        // Detect the faces
        std::pair<std::vector<cv::Rect>, double> detected_faces = detectFaces(frame);

        ++i;
        if (i % 10 == 0)
            std::cout << "time (sec): " << detected_faces.second << std::endl;

        for(const auto & rect : detected_faces.first) {
            rectangle(frame, cv::Point(rect.x, rect.y),
                      cv::Point(rect.x + rect.width, rect.y + rect.height),
                      cv::Scalar(255, 0, 0),
                      3, 8, 0);
        }

        cv::imshow("Head", frame);

        if (cv::pollKey() >= 0)
            break;
    }

    return 0;
}

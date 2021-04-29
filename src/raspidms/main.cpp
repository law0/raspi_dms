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
#include <vector>
#include <utility>

#include "TimeMark.hpp"

void printHelp () {
    std::cout << "raspidms PATH_TO_VIDEO.mp4 [haar|resnetCaffe]" << std::endl;
}

std::pair<std::vector<cv::Rect>, double> detectFacesHaar(cv::Mat frame) {
    static cv::CascadeClassifier face_cascade = cv::CascadeClassifier(cv::samples::findFile("haarcascades/haarcascade_frontalface_default.xml"));
    timeMark();
    cv::Mat frameCopy = frame.clone();
    // Convert to gray
    cv::cvtColor(frame, frameCopy, cv::COLOR_BGR2GRAY);

    std::vector<cv::Rect> faces;
    face_cascade.detectMultiScale(frameCopy, faces, 1.1, 5);

    return std::make_pair(faces, timeMark());
}

std::pair<std::vector<cv::Rect>, double> detectFacesResnetCaffe(cv::Mat frame) {
    static cv::dnn::Net net = cv::dnn::readNetFromCaffe("../res/Resnet_SSD_deploy.prototxt", "../res/Res10_300x300_SSD_iter_140000.caffemodel");
    double confThreshold = 0.5;
    timeMark();
    std::vector<cv::Rect> faces;

    cv::Mat frameCopy = frame.clone();
    cv::resize(frame, frameCopy, cv::Size(300, 300));

    cv::Mat blob;
    cv::dnn::blobFromImage(frameCopy, blob, 1.0, cv::Size(300, 300));
    net.setInput(blob);
    cv::Mat outs = net.forward();

    // Network produces output blob with a shape 1x1xNx7 where N is a number of
    // detections and an every detection is a vector of values
    // [batchId, classId, confidence, left, top, right, bottom]
    float* data = (float*)outs.data;
    for (size_t i = 0; i < outs.total(); i += 7)
    {
        float confidence = data[i + 2];
        if (confidence > confThreshold)
        {
            float left   = (float)data[i + 3];
            float top    = (float)data[i + 4];
            float right  = (float)data[i + 5];
            float bottom = (float)data[i + 6];
            float width  = right - left + 1;
            float height = bottom - top + 1;
            if (width <= 2 || height <= 2)
            {
                left   = (float)(data[i + 3] * frame.cols);
                top    = (float)(data[i + 4] * frame.rows);
                right  = (float)(data[i + 5] * frame.cols);
                bottom = (float)(data[i + 6] * frame.rows);
                width  = right - left + 1;
                height = bottom - top + 1;
            }
            faces.push_back(cv::Rect(left, top, width, height));
        }
    }

    return std::make_pair(faces, timeMark());
}

int main(int argc, char**argv) {
    if (argc < 3) {
        printHelp();
        return 0;
    }

    const std::string video_path(argv[1]);
    const std::string detector(argv[2]);

    std::pair<std::vector<cv::Rect>, double> (*detectFaces)(cv::Mat frame) = detectFacesHaar;

    if (detector == "haar") {
        detectFaces = detectFacesHaar;
    } else if (detector == "resnetCaffe") {
        detectFaces = detectFacesResnetCaffe;
    }


    cv::VideoCapture cap(video_path);
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

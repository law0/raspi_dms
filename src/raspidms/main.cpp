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

std::pair<std::vector<cv::Rect>, double> detectFacesYolo(const std::string& onnx_path , cv::Mat frame) {
    static cv::dnn::Net net = cv::dnn::readNetFromONNX(onnx_path);
    float confThreshold = 0.5;
    float classThreshold = 0.5;
    timeMark();
    std::vector<cv::Rect> faces;

    cv::Mat frameCopy = frame.clone();
    cv::resize(frame, frameCopy, cv::Size(224, 224));

    const cv::Scalar std_dev = cv::Scalar(0.229, 0.224, 0.225);

    cv::Mat blob;
    cv::dnn::blobFromImage(frameCopy, blob,
                           1. / 255., //pixels values between 0 and 1
                           cv::Size(224, 224),
                           cv::Scalar(0.485, 0.456, 0.406), //substract mean
                           true //swap Red Green
                           );

    // Divide blob by std.
    cv::divide(blob, std_dev, blob);

    net.setInput(blob);
    cv::Mat outs = net.forward();

    auto t = timeMark();

    // Network produces output blob with a shape 1x49x(1+4*5)
    // 49 Cells, and per cell : 1 class, 4 boxes, 1 confidence + 4 coords (x,y,w,h) per box.

    const float scale_x = 224. / frame.cols;
    const float scale_y = 224. / frame.rows;
    const int num_cells = 49; // 7 * 7 cells
    const int num_classes = 1;
    const int num_box_per_cells = 4;
    const int num_features_per_boxes = 5; //confidence, x, y, w, h
    float* data = (float*)outs.data;

    //Find the cell with max class
    float class_max = 0.0;
    size_t index_max = 0;
    size_t i_max = 0;
    for (size_t i = 0; i < num_cells; ++i)
    {
        size_t index = i * (num_classes + num_box_per_cells * num_features_per_boxes);
        float classe = data[index];
        if (classe > classThreshold && classe > class_max)
        {
            class_max = classe;
            index_max = index;
            i_max = i;
        }
    }

    int cell_x = floor(i_max % 7) * 32; // 224 / 7 = 32
    int cell_y = floor(i_max / 7) * 32;

    //in the max class cell, find the box with the max confidence
    float max_conf = 0.;
    size_t j_max = 0;
    for (size_t j = 0; j < num_box_per_cells * num_features_per_boxes; j += num_features_per_boxes)
    {
        float conf   = (float)data[index_max + j + 1];
        if (conf > confThreshold && conf > max_conf)
        {
            max_conf = conf;
            j_max = j;
        }
    }


    float width  = (float)data[index_max + j_max + 4];
    float height = (float)data[index_max + j_max + 5];

    width  = (float)(width * 224);
    height = (float)(height * 224);
    float left   = (float)(data[index_max + j_max + 2] * 32) + (float)cell_x - width / 2. ;
    float top    = (float)(data[index_max + j_max + 3] * 32) + (float)cell_y - height / 2. ;

    /*std::cout << "Class " << class_max << ", conf " << max_conf << ", ("
              << left << ", "
              << top << ", "
              << width << ", "
              << height << ")"
              << std::endl;*/

    faces.push_back(cv::Rect(left / scale_x, top / scale_y, width / scale_x, height / scale_y));

    return std::make_pair(faces, t);
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
    } else if (detector == "yoloResnet18") {
        detectFaces = [](cv::Mat frame) { return detectFacesYolo("../res/YoloResnet18.onnx", frame); };
    } else if (detector == "yoloEffnetb0") {
        detectFaces = [](cv::Mat frame) { return detectFacesYolo("../res/YoloEffnetb0.onnx", frame); };
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

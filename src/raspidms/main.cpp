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

#include <iostream>
#include <string>
#include <sys/stat.h>
#include <errno.h>
#include <vector>

static const std::string haarcascades_path = "/opt/opencv-4.5.2/share/opencv4/haarcascades/";

void printHelp () {
    std::cout << "raspidms PATH_TO_VIDEO.mp4" << std::endl;
}

int main(int argc, char**argv) {
    if (argc == 1) {
        printHelp();
    }

    const std::string video_path(argv[1]);

    // Load the cascade
    cv::CascadeClassifier face_cascade = cv::CascadeClassifier(haarcascades_path + "/haarcascade_frontalface_default.xml");
    cv::VideoCapture cap(video_path);
    if (! cap.isOpened()) {
        std::cerr << "Can't open file " << video_path << std::endl;
    }

    std::cout << "Start grabbing" << std::endl;

    cv::Mat frame;
    cv::namedWindow("Head", cv::WINDOW_AUTOSIZE);
    for(;;)
    {
        // wait for a new frame from camera and store it into 'frame'
        cap.read(frame);
        // check if we succeeded
        if (frame.empty()) {
            std::cerr << "ERROR! blank frame grabbed\n";
            break;
        }

        // Convert to gray
        cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);


        std::vector<cv::Rect> faces;
        // Detect the faces
        face_cascade.detectMultiScale(frame, faces, 1.1, 5);

        for(const auto & rect : faces) {
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


#include "DetectFacesHaar.h"

#include "TimeMark.hpp"

#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/core/utility.hpp>

DetectFacesHaar::DetectFacesHaar(const std::string & path) : IDetectFaces(path), m_faceCascade(cv::samples::findFile(m_path))
{

}

DetectedFacesResult DetectFacesHaar::operator()(cv::Mat frame) {
    std::cout << "haar" << std::endl;
    timeMark();
    cv::Mat frameCopy = frame.clone();
    // Convert to gray
    cv::cvtColor(frame, frameCopy, cv::COLOR_BGR2GRAY);

    std::vector<cv::Rect> faces;
    m_faceCascade.detectMultiScale(frameCopy, faces, 1.15, 5);

    return std::make_pair(faces, timeMark());
}

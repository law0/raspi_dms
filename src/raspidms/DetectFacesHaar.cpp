#include "DetectFacesHaar.h"

#include "TimeMark.hpp"

DetectFacesHaar::DetectFacesHaar(const std::string & path) : IDetectFaces(path)
{

}

std::pair<std::vector<cv::Rect>, double> DetectFacesHaar::operator()(cv::Mat frame) {
    static cv::CascadeClassifier face_cascade = cv::CascadeClassifier(cv::samples::findFile(m_path));
    timeMark();
    cv::Mat frameCopy = frame.clone();
    // Convert to gray
    cv::cvtColor(frame, frameCopy, cv::COLOR_BGR2GRAY);

    std::vector<cv::Rect> faces;
    face_cascade.detectMultiScale(frameCopy, faces, 1.15, 5);

    return std::make_pair(faces, timeMark());
}

#include "DetectFacesEmpty.h"

#include "TimeMark.hpp"

DetectFacesEmpty::DetectFacesEmpty(const std::string & path) : IDetectFaces(path)
{

}

DetectedFacesResult DetectFacesEmpty::operator()(cv::Mat frame) {
    timeMark();

    std::vector<cv::Rect> faces;

    return std::make_pair(faces, timeMark());
}

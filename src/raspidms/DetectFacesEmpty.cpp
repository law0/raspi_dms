#include "DetectFacesEmpty.h"

#include "TimeMark.h"

DetectFacesEmpty::DetectFacesEmpty(const std::string & path) : IDetectFaces(path), m_id(getUniqueId())
{

}

DetectedFacesResult DetectFacesEmpty::operator()(cv::Mat frame) {
    timeMark(m_id);

    std::vector<cv::Rect> faces;

    return std::make_pair(faces, timeMark(m_id));
}

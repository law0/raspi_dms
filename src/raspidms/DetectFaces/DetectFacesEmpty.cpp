#include "DetectFaces/DetectFacesEmpty.h"

#include "Utils.h"

DetectFacesEmpty::DetectFacesEmpty(const std::string & path) : IDetectFaces(path), m_id(getUniqueId())
{

}

DetectedFacesResult DetectFacesEmpty::operator()(const cv::Mat & frame) {
    timeMark(m_id);

    (void)frame;
    std::vector<cv::Rect> faces;
    std::cerr << "Warning: using DetectFacesEmpty" << std::endl;
    return std::make_pair(faces, timeMark(m_id));
}

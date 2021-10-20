#include "FaceFeatures/FaceFeaturesEmpty.h"
#include "Utils.h"

FaceFeaturesEmpty::FaceFeaturesEmpty(const std::string& path) : IFaceFeatures(path), m_id(getUniqueId())
{

}

FaceFeaturesResult FaceFeaturesEmpty::operator()(const cv::Mat& frame,
                                                std::vector<cv::Rect>& roi) {
    timeMark(m_id);
    std::vector<std::vector<cv::Point2i>> ret;
    std::cerr << "Warning: using FaceFeatureEmpty" << std::endl;
    return std::make_pair(ret, timeMark(m_id));
}

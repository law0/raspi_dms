#include "FaceFeatures/FaceFeaturesEmpty.h"
#include "Utils.h"

FaceFeaturesEmpty::FaceFeaturesEmpty(const std::string& path) : IFaceFeatures(path), m_id(getUniqueId())
{

}

PointsList FaceFeaturesEmpty::operator()(const cv::Mat& frame,
                                                std::vector<cv::Rect>& roi) {
    PointsList ret;
    std::cerr << "Warning: using FaceFeatureEmpty" << std::endl;
    return ret;
}

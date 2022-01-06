#include "DetectFaces/DetectFacesEmpty.h"

#include "Utils.h"

DetectFacesEmpty::DetectFacesEmpty(const std::string & path) : IDetectFaces(path), m_id(getUniqueId())
{

}

PointsList DetectFacesEmpty::operator()(const cv::Mat & frame) {

    (void)frame;
    PointsList faces;
    std::cerr << "Warning: using DetectFacesEmpty" << std::endl;
    return faces;
}

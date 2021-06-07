#ifndef DETECTFACESEMPTY_H
#define DETECTFACESEMPTY_H

#include "IDetectFaces.h"

class DetectFacesEmpty : public IDetectFaces
{
public:
    DetectFacesEmpty(const std::string & path = std::string());
    std::pair<std::vector<cv::Rect>, double> operator()(cv::Mat frame);
};

#endif // DETECTFACESEMPTY_H

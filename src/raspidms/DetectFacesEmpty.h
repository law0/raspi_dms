#ifndef DETECTFACESEMPTY_H
#define DETECTFACESEMPTY_H

#include "IDetectFaces.h"

class DetectFacesEmpty : public IDetectFaces
{
public:
    DetectFacesEmpty(const std::string & path = std::string());
    DetectedFacesResult operator()(cv::Mat frame);
};

#endif // DETECTFACESEMPTY_H

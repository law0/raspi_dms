#ifndef DETECTFACESEMPTY_H
#define DETECTFACESEMPTY_H

#include "DetectFaces/IDetectFaces.h"

class DetectFacesEmpty : public IDetectFaces
{
public:
    DetectFacesEmpty(const std::string & path = std::string());
    virtual DetectedFacesResult operator()(const cv::Mat & frame) override;

private:
    const long m_id;
};

#endif // DETECTFACESEMPTY_H

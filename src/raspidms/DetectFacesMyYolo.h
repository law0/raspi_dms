#ifndef DETECTFACESMYYOLO_H
#define DETECTFACESMYYOLO_H

#include "IDetectFaces.h"

class DetectFacesMyYolo : public IDetectFaces
{
public:
    DetectFacesMyYolo(const std::string & path);
    virtual DetectedFacesResult operator()(const cv::Mat & frame) override;

private:
    //warning dnn::Net seems not reentrant
    cv::dnn::Net m_net;
    const long m_id;
};

#endif // DETECTFACESMYYOLO_H

#ifndef DETECTFACESMYYOLO_H
#define DETECTFACESMYYOLO_H

#include "DetectFaces/IDetectFaces.h"

class DetectFacesMyYolo : public IDetectFaces
{
public:
    DetectFacesMyYolo(const std::string & path);
    virtual PointsList operator()(const cv::Mat & frame) override;

private:
    //warning dnn::Net seems not reentrant
    cv::dnn::Net m_net;
    const long m_id;
};

#endif // DETECTFACESMYYOLO_H

#ifndef DETECTFACESMYYOLO_H
#define DETECTFACESMYYOLO_H

#include "IDetectFaces.h"

class DetectFacesMyYolo : public IDetectFaces
{
public:
    DetectFacesMyYolo(const std::string & path);
   DetectedFacesResult operator()(cv::Mat frame);

private:

    //warning dnn::Net seems not reentrant
    cv::dnn::Net m_net;
};

#endif // DETECTFACESMYYOLO_H

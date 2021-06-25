#ifndef DETECTFACESMYYOLO_H
#define DETECTFACESMYYOLO_H

#include "IDetectFaces.h"

class DetectFacesMyYolo : public IDetectFaces
{
public:
    DetectFacesMyYolo(const std::string & path);
    std::pair<std::vector<cv::Rect>, double> operator()(cv::Mat frame);

private:
    cv::dnn::Net m_net;
};

#endif // DETECTFACESMYYOLO_H

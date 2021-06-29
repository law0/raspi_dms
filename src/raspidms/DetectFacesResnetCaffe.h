#ifndef DETECTFACESRESNETCAFFE_H
#define DETECTFACESRESNETCAFFE_H

#include "IDetectFaces.h"

class DetectFacesResnetCaffe : public IDetectFaces
{
public:
    DetectFacesResnetCaffe(const std::string & protoTxtPath, const std::string & caffeModelPath);
    DetectedFacesResult operator()(cv::Mat frame);

private:

    //warning dnn::Net seems not reentrant
    cv::dnn::Net m_net;
};

#endif // DETECTFACESRESNETCAFFE_H

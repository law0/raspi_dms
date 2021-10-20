#ifndef DETECTFACESRESNETCAFFE_H
#define DETECTFACESRESNETCAFFE_H

#include "DetectFaces/IDetectFaces.h"

class DetectFacesResnetCaffe : public IDetectFaces
{
public:
    DetectFacesResnetCaffe(const std::string & protoTxtPath, const std::string & caffeModelPath);
    virtual DetectedFacesResult operator()(const cv::Mat & frame) override;

private:

    //warning dnn::Net seems not reentrant
    cv::dnn::Net m_net;
    const long m_id;
};

#endif // DETECTFACESRESNETCAFFE_H

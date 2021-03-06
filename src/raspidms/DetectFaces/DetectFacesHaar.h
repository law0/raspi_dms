#ifndef DETECTFACESHAAR_H
#define DETECTFACESHAAR_H

#include "DetectFaces/IDetectFaces.h"

class DetectFacesHaar : public IDetectFaces
{
public:
    DetectFacesHaar(const std::string & path);
    virtual PointsList operator()(const cv::Mat & frame) override;

private:

    //warning maybe not reentrant
    cv::CascadeClassifier m_faceCascade;
    const long m_id;
};

#endif // DETECTFACESHAAR_H

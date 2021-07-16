#ifndef DETECTFACESHAAR_H
#define DETECTFACESHAAR_H

#include "IDetectFaces.h"

class DetectFacesHaar : public IDetectFaces
{
public:
    DetectFacesHaar(const std::string & path);
    DetectedFacesResult operator()(const cv::Mat & frame);

private:

    //warning maybe not reentrant
    cv::CascadeClassifier m_faceCascade;
    const long m_id;
};

#endif // DETECTFACESHAAR_H

#ifndef DETECTFACESHAAR_H
#define DETECTFACESHAAR_H

#include "IDetectFaces.h"

class DetectFacesHaar : public IDetectFaces
{
public:
    DetectFacesHaar(const std::string & path);
    std::pair<std::vector<cv::Rect>, double> operator()(cv::Mat frame);

private:
    cv::CascadeClassifier m_faceCascade;
};

#endif // DETECTFACESHAAR_H

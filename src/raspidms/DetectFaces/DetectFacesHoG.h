#ifndef DETECTFACESHOG_H
#define DETECTFACESHOG_H

#include "DetectFaces/IDetectFaces.h"

#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>

#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/core/utility.hpp>

class DetectFacesHoG : public IDetectFaces
{
public:
    DetectFacesHoG(const std::string & path = std::string());
    virtual PointsList operator()(const cv::Mat & frame) override;

private:

    //warning maybe not reentrant
    dlib::frontal_face_detector m_frontalFaceDetector;
    const long m_id;
};

#endif // DETECTFACESHOG_H

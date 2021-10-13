#ifndef FACEFEATURESDLIB_H
#define FACEFEATURESDLIB_H

#include "IFaceFeatures.h"
#include <dlib/image_processing.h>

class FaceFeaturesDlib : public IFaceFeatures
{
public:
    FaceFeaturesDlib(const std::string & path);

    virtual FaceFeaturesResult operator()(const cv::Mat & frame,
                                          std::vector<cv::Rect>& roi) override;


private:
    dlib::shape_predictor m_shapePredictor;
    const long m_id;
};

#endif // FACEFEATURESDLIB_H

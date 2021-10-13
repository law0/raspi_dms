#ifndef FACEFEATUREEMPTY_H
#define FACEFEATUREEMPTY_H

#include "IFaceFeatures.h"

class FaceFeatureEmpty : public IFaceFeatures
{
public:
    FaceFeatureEmpty(const std::string& path = std::string());

    virtual FaceFeaturesResult operator()(const cv::Mat & frame,
                                          std::vector<cv::Rect>& roi) override;
private:
    const long m_id;
};

#endif // FACEFEATUREEMPTY_H

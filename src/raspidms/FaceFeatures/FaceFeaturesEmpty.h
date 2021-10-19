#ifndef FACEFEATURESEMPTY_H
#define FACEFEATURESEMPTY_H

#include "FaceFeatures/IFaceFeatures.h"

class FaceFeaturesEmpty : public IFaceFeatures
{
public:
    FaceFeaturesEmpty(const std::string& path = std::string());

    virtual FaceFeaturesResult operator()(const cv::Mat & frame,
                                          std::vector<cv::Rect>& roi) override;
private:
    const long m_id;
};

#endif // FACEFEATURESEMPTY_H

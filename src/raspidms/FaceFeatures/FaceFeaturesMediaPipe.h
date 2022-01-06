#ifndef FACEFEATURESMEDIAPIPE_H
#define FACEFEATURESMEDIAPIPE_H

#include "FaceFeatures/IFaceFeatures.h"

#include "tensorflow/lite/interpreter.h"


class FaceFeaturesMediaPipe : public IFaceFeatures
{
public:
    FaceFeaturesMediaPipe(const std::string & path);
    virtual PointsList operator()(const cv::Mat & frame,
                                          std::vector<cv::Rect>& roi) override;

    void printModelIOTensorsInfo();

private:
    std::unique_ptr<tflite::Interpreter> m_interpreter;
    const long m_id;
};

#endif // FACEFEATURESMEDIAPIPE_H

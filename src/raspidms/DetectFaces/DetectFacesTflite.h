#ifndef DETECTFACESTFLITE_H
#define DETECTFACESTFLITE_H

#include <memory>

#include "DetectFaces/IDetectFaces.h"

#include "tensorflow/lite/interpreter.h"

class DetectFacesTflite : public IDetectFaces
{
public:
    DetectFacesTflite(const std::string & modelPath);
    virtual DetectedFacesResult operator()(const cv::Mat & frame) override;

private:
    const long m_id;
    std::unique_ptr<tflite::Interpreter> m_interpreter;
};

#endif // DETECTFACESTFLITE_H

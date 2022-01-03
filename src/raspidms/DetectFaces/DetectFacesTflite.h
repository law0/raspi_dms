#ifndef DETECTFACESTFLITE_H
#define DETECTFACESTFLITE_H

#include <memory>
#include <vector>

#include "DetectFaces/IDetectFaces.h"

#include "tensorflow/lite/interpreter.h"

class DetectFacesTflite : public IDetectFaces
{
public:
    DetectFacesTflite(const std::string & modelPath);
    virtual DetectedFacesResult operator()(const cv::Mat & frame) override;

    void printModelIOTensorsInfo();

private:
    void generate_anchors();

    const long m_id;
    std::unique_ptr<tflite::Interpreter> m_interpreter;
    std::vector<std::vector<float>> m_anchors;
};

#endif // DETECTFACESTFLITE_H

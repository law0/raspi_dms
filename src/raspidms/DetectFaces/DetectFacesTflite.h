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
    virtual PointsList operator()(const cv::Mat & frame) override;

    void printModelIOTensorsInfo();

private:
    void generate_anchors();
    float iou_score(const cv::Rect& a, const cv::Rect& b) const;

    const long m_id;
    std::unique_ptr<tflite::Interpreter> m_interpreter;
    std::vector<std::vector<float>> m_anchors;
};

#endif // DETECTFACESTFLITE_H

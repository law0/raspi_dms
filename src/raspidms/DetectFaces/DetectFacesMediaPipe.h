#ifndef DETECTFACESMEDIAPIPE_H
#define DETECTFACESMEDIAPIPE_H

#include <memory>
#include <vector>

#include "DetectFaces/IDetectFaces.h"

#include "tensorflow/lite/interpreter.h"

class DetectFacesMediaPipe : public IDetectFaces
{
public:
    DetectFacesMediaPipe(const std::string & modelPath);
    virtual PointsList operator()(const cv::Mat & frame) override;

    void printModelIOTensorsInfo();

private:
    void generate_anchors();
    void filterSimilarIOU(PointsList& faces, float threshold) const;

    const long m_id;
    std::unique_ptr<tflite::Interpreter> m_interpreter;
    std::vector<std::vector<float>> m_anchors;
};

#endif // DETECTFACESMEDIAPIPE_H

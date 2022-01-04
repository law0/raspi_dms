#ifndef DETECTFACESSTAGE_H
#define DETECTFACESSTAGE_H

#include <unordered_map>
#include <mutex>
#include <string>
#include <utility>

#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/core/utility.hpp>

#include "DetectFaces/IDetectFaces.h"
#include "FaceFeatures/IFaceFeatures.h"
#include "IStage.h"
#include "SharedQueue.h"

const std::string HAAR_CASCADE_PATH = "haarcascades/haarcascade_frontalface_default.xml";
const std::string RESNET_CAFFE_PROTO_TXT_PATH = "../res/Resnet_SSD_deploy.prototxt";
const std::string RESNET_CAFFE_MODEL_PATH = "../res/Res10_300x300_SSD_iter_140000.caffemodel";
const std::string MY_YOLO_RESNET_18_PATH = "../res/YoloResnet18.onnx";
const std::string MY_YOLO_EFFNET_B0_PATH = "../res/YoloEffnetb0.onnx";
const std::string TF_LITE_MODEL_PATH = "../res/face_detection_short_range.tflite";

class DetectFacesStage : public IStage
{

public:
    DetectFacesStage(const std::string& detectorName,
                     std::shared_ptr<SharedQueue<cv::Mat>> inFrames,
                     std::shared_ptr<SharedQueue<PointsList>> outRects);
    DetectFacesStage(const DetectFacesStage&) = delete;

    /**
     * override void IStage::operator()(int);
     */
    virtual void operator()(int) override;

    /**
     * override double IStage::averageTime();
     */
    virtual double averageTime() override;

private:
    /**
     * @brief getNextDetector
     * @param threadId
     * @return next detector, scheduled by schedNextDetector, and specialized by threadId (one threadId for one detector)
     */
    std::shared_ptr<IDetectFaces> getNextDetector(int threadId);

    const std::string m_detectorName;
    std::shared_ptr<SharedQueue<cv::Mat>> m_inFrames;
    std::shared_ptr<SharedQueue<PointsList>> m_outRects;
    std::unordered_map<int /*threadId*/, std::shared_ptr<IDetectFaces>> m_detectors;
    std::mutex m_mutex;
    double m_averageTime;
    double m_averageAlpha;
};

#endif // DETECTFACESSTAGE_H

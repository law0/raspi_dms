#ifndef DETECTFACESSTAGE_H
#define DETECTFACESSTAGE_H

#include <map>
#include <mutex>
#include <string>
#include <utility>

#include "DetectFacesEmpty.h"
#include "DetectFacesHaar.h"
#include "DetectFacesMyYolo.h"
#include "DetectFacesResnetCaffe.h"
#include "DetectFacesHoG.h"

#include "IDetectFaces.h"
#include "SharedQueue.h"

const int OUT_RECT_BUFFER_SIZE = 4;
const int MAX_CONCURRENT_DETECTORS = 4;

const std::string HAAR_CASCADE_PATH = "haarcascades/haarcascade_frontalface_default.xml";
const std::string RESNET_CAFFE_PROTO_TXT_PATH = "../res/Resnet_SSD_deploy.prototxt";
const std::string RESNET_CAFFE_MODEL_PATH = "../res/Res10_300x300_SSD_iter_140000.caffemodel";
const std::string MY_YOLO_RESNET_18_PATH = "../res/YoloResnet18.onnx";
const std::string MY_YOLO_EFFNET_B0_PATH = "../res/YoloEffnetb0.onnx";

class DetectFacesStage {

public:
    DetectFacesStage(std::shared_ptr<SharedQueue<cv::Mat>> inFrames, std::shared_ptr<SharedQueue<DetectedFacesResult>> outRects);
    DetectFacesStage(const DetectFacesStage&) = delete;

    /**
     * @brief operator ()
     * to be called by the ThreadPool. The whole DetectFacesStage object can be pushed
     * as threadPool.push(std::ref(detectFacesStage))
     * This function purposely terminates, and so needs to be repushed regularly
     */
    void operator()(int);

    /**
     * @brief schedNextDetector
     * schedule the next detector to be run
     */
    void schedNextDetector(const std::string &);

private:
    /**
     * @brief getNextDetector
     * @param threadId
     * @return next detector, scheduled by schedNextDetector, and specialized by threadId (one threadId for one detector)
     */
    std::shared_ptr<IDetectFaces> getNextDetector(int threadId);

    std::shared_ptr<SharedQueue<cv::Mat>> m_inFrames;
    std::shared_ptr<SharedQueue<DetectedFacesResult>> m_outRects;
    std::map<std::pair<int /*threadId*/, std::string /*name*/>, std::shared_ptr<IDetectFaces>> m_detectors;
    SharedQueue<std::string> m_schedQueue;
    std::mutex m_mutex;
};

#endif // DETECTFACESSTAGE_H

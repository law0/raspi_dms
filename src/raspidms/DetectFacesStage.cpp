#include "DetectFacesStage.h"

DetectFacesStage::DetectFacesStage(std::shared_ptr<SharedQueue<cv::Mat>> inFrames, std::shared_ptr<SharedQueue<DetectedFacesResult>> outRects)
    : m_inFrames(inFrames),
      m_outRects(outRects),
      m_detectors(),
      m_schedQueue(),
      m_mutex()
{

}

void DetectFacesStage::operator()(int threadId) {
    std::cout << "thread id " << threadId << std::endl;

    std::shared_ptr<IDetectFaces> detector = getNextDetector(threadId);

    //avoid calculus completely if outFrames buffer is full
    if (m_outRects->size() >= OUT_RECT_BUFFER_SIZE)
        return;

    //TODO consider no copy
    //Don't wait for frame, there should be plenty
    cv::Mat frame;
    if(! m_inFrames->pop_front_no_wait(frame))
        return;

    // Detect the faces
    m_outRects->push_back((*detector)(frame));
}

void DetectFacesStage::schedNextDetector(const std::string & name) {
    if (m_schedQueue.size() >= MAX_CONCURRENT_DETECTORS)
        return;
    m_schedQueue.push_back(name);
}

std::shared_ptr<IDetectFaces> DetectFacesStage::getNextDetector(int threadId) {
    std::string detectorName;
    if (! m_schedQueue.pop_front_no_wait(detectorName)) {
        return std::make_shared<DetectFacesEmpty>("");
    }

    std::lock_guard<std::mutex> guard(m_mutex);

    auto detectorIt = m_detectors.find(std::make_pair(threadId, detectorName));
    if (detectorIt != m_detectors.end()) {
        return detectorIt->second;
    } else {
        std::shared_ptr<IDetectFaces> detector;
        if (detectorName == "haar")
        {
            detector.reset(new DetectFacesHaar(HAAR_CASCADE_PATH));
            m_detectors.insert({{threadId, detectorName}, detector});
        }
        else if (detectorName == "resnetCaffe")
        {
            detector.reset(new DetectFacesResnetCaffe(RESNET_CAFFE_PROTO_TXT_PATH, RESNET_CAFFE_MODEL_PATH));
            m_detectors.insert({{threadId, detectorName}, detector});
        }
        else if (detectorName == "yoloResnet18")
        {
            detector.reset(new DetectFacesMyYolo(MY_YOLO_RESNET_18_PATH));
            m_detectors.insert({{threadId, detectorName}, detector});
        }
        else if (detectorName == "yoloEffnetb0")
        {
            detector.reset(new DetectFacesMyYolo(MY_YOLO_EFFNET_B0_PATH));
            m_detectors.insert({{threadId, detectorName}, detector});
        }
        else if (detectorName == "hog")
        {
            detector.reset(new DetectFacesHoG());
            m_detectors.insert({{threadId, detectorName}, detector});
        }
        else if (detectorName == "empty")
        {
            detector.reset(new DetectFacesEmpty());
            m_detectors.insert({{threadId, detectorName}, detector});
        }
        else //do not insert detector if name unkown, but still return empty one
        {
            detector.reset(new DetectFacesEmpty());
        }

        return detector;
    }
}

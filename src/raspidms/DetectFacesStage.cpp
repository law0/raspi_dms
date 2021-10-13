#include "DetectFacesStage.h"

#include "DetectFacesEmpty.h"
#include "DetectFacesHaar.h"
#include "DetectFacesMyYolo.h"
#include "DetectFacesResnetCaffe.h"
#include "DetectFacesHoG.h"

DetectFacesStage::DetectFacesStage(const std::string& detectorName,
                                   std::shared_ptr<SharedQueue<cv::Mat>> inFrames,
                                   std::shared_ptr<SharedQueue<DetectedFacesResult>> outRects)
    : m_detectorName(detectorName),
      m_inFrames(inFrames),
      m_outRects(outRects),
      m_detectors(),
      m_mutex()
{

}

void DetectFacesStage::operator()(int threadId) {
    //std::cout << "thread id " << threadId << std::endl;
    std::shared_ptr<IDetectFaces> detector = getNextDetector(threadId);

    //Don't wait for frame, there should be plenty
    cv::Mat frame;
    if(! m_inFrames->front_no_wait(frame)) {
        std::cout << __FUNCTION__ << ": " << "empty frame" << std::endl;
        return;
    }

    // Detect the faces
    m_outRects->push_back((*detector)(frame));
}

std::shared_ptr<IDetectFaces> DetectFacesStage::getNextDetector(int threadId) {
    std::lock_guard<std::mutex> guard(m_mutex);

    auto detectorIt = m_detectors.find(threadId);
    if (detectorIt != m_detectors.end()) {
        return detectorIt->second;
    } else {
        std::shared_ptr<IDetectFaces> detector;
        if (m_detectorName == "haar")
        {
            detector.reset(new DetectFacesHaar(HAAR_CASCADE_PATH));
            m_detectors.insert({threadId , detector});
        }
        else if (m_detectorName == "resnetCaffe")
        {
            detector.reset(new DetectFacesResnetCaffe(RESNET_CAFFE_PROTO_TXT_PATH, RESNET_CAFFE_MODEL_PATH));
            m_detectors.insert({threadId, detector});
        }
        else if (m_detectorName == "yoloResnet18")
        {
            detector.reset(new DetectFacesMyYolo(MY_YOLO_RESNET_18_PATH));
            m_detectors.insert({threadId, detector});
        }
        else if (m_detectorName == "yoloEffnetb0")
        {
            detector.reset(new DetectFacesMyYolo(MY_YOLO_EFFNET_B0_PATH));
            m_detectors.insert({threadId, detector});
        }
        else if (m_detectorName == "hog")
        {
            detector.reset(new DetectFacesHoG());
            m_detectors.insert({threadId, detector});
        }
        else if (m_detectorName == "empty")
        {
            detector.reset(new DetectFacesEmpty());
            m_detectors.insert({threadId, detector});
        }
        else //do not insert detector if name unkown, but still return empty one
        {
            detector.reset(new DetectFacesEmpty());
        }

        return detector;
    }
}

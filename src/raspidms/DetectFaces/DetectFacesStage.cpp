#include "DetectFaces/DetectFacesStage.h"

#include "DetectFaces/DetectFacesEmpty.h"
#include "DetectFaces/DetectFacesHaar.h"
#include "DetectFaces/DetectFacesMyYolo.h"
#include "DetectFaces/DetectFacesResnetCaffe.h"
#include "DetectFaces/DetectFacesHoG.h"
#include "DetectFaces/DetectFacesMediaPipe.h"

#include "Utils.h"

const size_t MAX_OUT_QUEUE_SIZE = 4;
const double INITIAL_AVERAGE_TIME = 0.5;
const double AVERAGE_ALPHA = 0.1;

DetectFacesStage::DetectFacesStage(const std::string& detectorName,
                                   std::shared_ptr<SharedQueue<cv::Mat>> inFrames,
                                   std::shared_ptr<SharedQueue<PointsList>> outRects)
    : m_detectorName(detectorName),
      m_inFrames(inFrames),
      m_outRects(outRects),
      m_detectors(),
      m_mutex(),
      m_averageTime(INITIAL_AVERAGE_TIME),
      m_averageAlpha(AVERAGE_ALPHA)
{

}

void DetectFacesStage::operator()(int threadId) {
    std::cout << "----> DetectFacesStage thread id " << threadId << std::endl;
    std::shared_ptr<IDetectFaces> detector = getNextDetector(threadId);

    cv::Mat frame;
    // try pop without emptying
    if (m_inFrames->size() > 1) {
        m_inFrames->pop_front_wait(frame);
    } else {
        frame = m_inFrames->front_wait();
    }

    if (frame.empty()) {
        std::cout << "DetectFacesStage: " << "empty frame" << std::endl;
        return;
    }

    timeMark(threadId);
    // Detect the faces
    PointsList pl = (*detector)(frame);

    // Exponential moving average
    m_averageTime = m_averageAlpha * timeMark(threadId) + (1. - m_averageAlpha) * m_averageTime;

    std::cout << "DetectFacesStage average time: " << m_averageTime << std::endl;

    m_outRects->push_back(pl);

    if (m_outRects->size() > MAX_OUT_QUEUE_SIZE)
        m_outRects->pop_front_no_wait();
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
        else if (m_detectorName == "mediapipe")
        {
            detector.reset(new DetectFacesMediaPipe(MEDIAPIPE_FD_MODEL_PATH));
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

double DetectFacesStage::averageTime() {
    return m_averageTime;
}

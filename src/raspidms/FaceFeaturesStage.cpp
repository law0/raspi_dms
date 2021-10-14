#include "FaceFeaturesDlib.h"
#include "FaceFeatureEmpty.h"
#include "FaceFeaturesStage.h"

const size_t MAX_OUT_QUEUE_SIZE = 2;

FaceFeaturesStage::FaceFeaturesStage(const std::string& detectorName,
                                     std::shared_ptr<SharedQueue<cv::Mat>> inFrames,
                                     std::shared_ptr<SharedQueue<DetectedFacesResult>> regionOfInterests,
                                     std::shared_ptr<SharedQueue<FaceFeaturesResult>> outFaceFeatures)
    : m_detectorName(detectorName),
      m_inFrames(inFrames),
      m_regionOfInterests(regionOfInterests),
      m_outFaceFeatures(outFaceFeatures),
      m_mutex()
{

}

void FaceFeaturesStage::operator()(int threadId) {
    std::cout << "FaceFeaturesStage thread id " << threadId << std::endl;
    std::shared_ptr<IFaceFeatures> detector = getNextDetector(threadId);

    //Don't wait for frame, there should be plenty
    cv::Mat frame;
    if (!m_inFrames->front_no_wait(frame)) {
        std::cout << __FUNCTION__ << ": " << "empty frame" << std::endl;
        return;
    }

    DetectedFacesResult dfr;
    if(! m_regionOfInterests->front_no_wait(dfr) || dfr.first.size() == 0) {
        std::cout << __FUNCTION__ << ": " << "empty RoI" << std::endl;
        return;
    }

    // Detect the faces features
    m_outFaceFeatures->push_back((*detector)(frame, dfr.first));

    if (m_outFaceFeatures->size() > MAX_OUT_QUEUE_SIZE)
        m_outFaceFeatures->pop_front_no_wait();
}


std::shared_ptr<IFaceFeatures> FaceFeaturesStage::getNextDetector(int threadId) {
    std::lock_guard<std::mutex> guard(m_mutex);

    auto detectorIt = m_detectors.find(threadId);
    if (detectorIt != m_detectors.end()) {
        return detectorIt->second;
    } else {
        std::shared_ptr<IFaceFeatures> detector;
        if (m_detectorName == "dlib_68")
        {
            detector.reset(new FaceFeaturesDlib(DLIB_68_FACE_LANDMARKS_PATH));
            m_detectors.insert({threadId, detector});
        }
        else //do not insert detector if name unkown, but still return empty one
        {
            detector.reset(new FaceFeatureEmpty());
        }

        return detector;
    }
}

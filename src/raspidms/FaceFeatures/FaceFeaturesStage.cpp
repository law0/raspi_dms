#include "FaceFeatures/FaceFeaturesDlib.h"
#include "FaceFeatures/FaceFeaturesEmpty.h"
#include "FaceFeatures/FaceFeaturesStage.h"

#include "Utils.h"

const size_t MAX_OUT_QUEUE_SIZE = 2;
const double INITIAL_AVERAGE_TIME = 0.1;
const double AVERAGE_ALPHA = 0.1;

FaceFeaturesStage::FaceFeaturesStage(const std::string& detectorName,
                                     std::shared_ptr<SharedQueue<cv::Mat>> inFrames,
                                     std::shared_ptr<SharedQueue<PointsList>> regionOfInterests,
                                     std::shared_ptr<SharedQueue<PointsList>> outFaceFeatures)
    : m_detectorName(detectorName),
      m_inFrames(inFrames),
      m_regionOfInterests(regionOfInterests),
      m_outFaceFeatures(outFaceFeatures),
      m_mutex(),
      m_averageTime(INITIAL_AVERAGE_TIME),
      m_averageAlpha(AVERAGE_ALPHA)
{

}

void FaceFeaturesStage::operator()(int threadId) {
    std::cout << "FaceFeaturesStage thread id " << threadId << std::endl;
    std::shared_ptr<IFaceFeatures> detector = getNextDetector(threadId);

    //Don't wait for frame, there should be plenty
    cv::Mat frame;
    if (!m_inFrames->pop_front_no_wait(frame) || frame.empty()) {
        std::cout << "FaceFeaturesStage: " << "empty frame" << std::endl;
        return;
    }

    PointsList pl_rois;
    if(! m_regionOfInterests->front_no_wait(pl_rois)) {
        std::cout << "FaceFeaturesStage: " << "RoI queue empty: " << m_regionOfInterests->size() << std::endl;
        return;
    }

    if (pl_rois.size() == 0 || pl_rois[0].size() < 2) {
        std::cout << "FaceFeaturesStage: " << "Void RoI ! This should not happen !" << std::endl;

        // Normally no empty RoI should be pushed into the RoI queue !

        // No need for a void region of interest
        //m_regionOfInterests->pop_front_no_wait();

    } else {

        std::vector<cv::Rect> rois;
        for (auto& head : pl_rois) {
            rois.push_back(cv::Rect(head[0], head[1]));
        }

        timeMark(threadId);
        // Detect the faces features
        PointsList faces_features = (*detector)(frame, rois);

        // Exponential moving average
        m_averageTime = m_averageAlpha * timeMark(threadId) + (1. - m_averageAlpha) * m_averageTime;


        if (faces_features.size() == 0) {
            std::cout << "FaceFeaturesStage: " << "No face feature detected" << std::endl;
        } else {
            m_outFaceFeatures->push_back(faces_features);
        }
    }


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
            detector.reset(new FaceFeaturesEmpty());
        }

        return detector;
    }
}

double FaceFeaturesStage::averageTime() {
    return m_averageTime;
}

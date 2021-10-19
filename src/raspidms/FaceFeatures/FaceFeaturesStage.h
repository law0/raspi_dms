#ifndef FACEFEATURESSTAGE_H
#define FACEFEATURESSTAGE_H

#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/core/utility.hpp>

#include <unordered_map>
#include <vector>

#include "SharedQueue.h"
#include "FaceFeatures/IFaceFeatures.h"
#include "DetectFaces/IDetectFaces.h"
#include "IStage.h"

const std::string DLIB_68_FACE_LANDMARKS_PATH = "../res/shape_predictor_68_face_landmarks.dat";

class FaceFeaturesStage : public IStage
{
public:
    FaceFeaturesStage(const std::string& detectorName,
                      std::shared_ptr<SharedQueue<cv::Mat>> inFrames,
                      std::shared_ptr<SharedQueue<DetectedFacesResult>> regionOfInterests,
                      std::shared_ptr<SharedQueue<FaceFeaturesResult>> outFaceFeatures);
    FaceFeaturesStage(const FaceFeaturesStage&) = delete;

    /**
     * override void IStage::operator()(int);
     */
    virtual void operator()(int threadId) override;

private:
    /**
     * @brief getNextDetector
     * @param threadId
     * @return next detector, scheduled by schedNextDetector, and specialized by threadId (one threadId for one detector)
     */
    std::shared_ptr<IFaceFeatures> getNextDetector(int threadId);

    const std::string m_detectorName;
    std::shared_ptr<SharedQueue<cv::Mat>> m_inFrames;
    std::shared_ptr<SharedQueue<DetectedFacesResult>> m_regionOfInterests;
    std::shared_ptr<SharedQueue<FaceFeaturesResult>> m_outFaceFeatures;
    std::unordered_map<int /*threadId*/, std::shared_ptr<IFaceFeatures>> m_detectors;
    std::mutex m_mutex;

};

#endif // FACEFEATURESSTAGE_H

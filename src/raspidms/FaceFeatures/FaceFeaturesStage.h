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
const std::string MEDIAPIPE_FACE_LANDMARKS_PATH = "../res/face_landmark.tflite";

class FaceFeaturesStage : public IStage
{
public:
    FaceFeaturesStage(const std::string& detectorName,
                      std::shared_ptr<SharedQueue<cv::Mat>> inFrames,
                      std::shared_ptr<SharedQueue<PointsList>> regionOfInterests,
                      std::shared_ptr<SharedQueue<PointsList>> outFaceFeatures);
    FaceFeaturesStage(const FaceFeaturesStage&) = delete;

    /**
     * override void IStage::operator()(int);
     */
    virtual void operator()(int threadId) override;

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
    std::shared_ptr<IFaceFeatures> getNextDetector(int threadId);

    const std::string m_detectorName;
    std::shared_ptr<SharedQueue<cv::Mat>> m_inFrames;
    std::shared_ptr<SharedQueue<PointsList>> m_regionOfInterests;
    std::shared_ptr<SharedQueue<PointsList>> m_outFaceFeatures;
    PointsList m_lastValidRoi;
    std::unordered_map<int /*threadId*/, std::shared_ptr<IFaceFeatures>> m_detectors;
    std::mutex m_mutex;
    double m_averageTime;
    double m_averageAlpha;

};

#endif // FACEFEATURESSTAGE_H

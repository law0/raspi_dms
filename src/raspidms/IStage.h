#ifndef ISTAGE_H
#define ISTAGE_H

#include <opencv2/core/types.hpp>

#include <vector>

/**
 * @brief PointsList is the generic return type for detection stages
 * A cv::Point2f will represent a point in the frame
 * Bounding box will be returned via this type with the convention : {top-left-point, bottom-right-point}
 * Note that other points may be returned after a bounding box,
 * example {top-left-point, bottom-right-point, right-eye-point, left-eye-point, etc.}
 */
typedef std::vector<std::vector<cv::Point2f>> PointsList;

class IStage {
public:
    virtual ~IStage() {}

    /**
     * @brief operator ()
     * to be called by the Scheduler/ThreadPool. The whole object must be pushable
     * as threadPool.push(std::ref(object))
     * This function must purposely terminates, and so needs to be repushed regularly
     */
    virtual void operator()(int) = 0;

    /**
     * @brief averageTime
     * @return the averageTime (in seconds) it takes to complete this stage
     */
    virtual double averageTime() = 0;
};

#endif // ISTAGE_H

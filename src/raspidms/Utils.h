#ifndef UTILS_H
#define UTILS_H

#include <opencv2/opencv.hpp>

#include <dlib/geometry/rectangle.h>

#include <map>
#include <mutex>
/**
 * @brief timeMark
 * @param id
 * @param update, set to false if you don't want to update the internal time upon call of timeMark
 * @return For a same id given, time in seconds since boot the first time, time since last call the next times
 */
inline double timeMark(long id=0, bool update=true) {
    static std::map<long, double> timeMap;
    static std::mutex mutex;

    {
        std::lock_guard<std::mutex> guard(mutex);
        if (timeMap.count(id) == 0) {
            timeMap.insert({id, 0.0});
        }
    }

    double ret = 0;
    double newTickCount = static_cast<double>(cv::getTickCount());
    ret = (newTickCount - timeMap[id])/cv::getTickFrequency();
    if (update)
        timeMap[id] = newTickCount;
    return ret;
}

/**
 * @brief getUniqueId
 * @return a unique id each time it is called (increment), useful to call timeMark later on
 */
inline long getUniqueId() {
    static long t = 0;
    return t++;
}

/**
 * @brief dlibRectangleToOpenCV
 * @param r dlib rectangle
 * @return an OpenCV rectangle converted from the dlib rectangle passed as param
 */
inline cv::Rect dlibRectangleToOpenCV(const dlib::rectangle & r)
{
    return cv::Rect(cv::Point2f(r.left(), r.top()), cv::Point2f(r.right() + 1, r.bottom() + 1));
}

/**
 * @brief openCVRectangleToDlib
 * @param r OpenCV rectangle
 * @return an dlib rectangle converted from the OpenCV rectangle passed as param
 */
inline dlib::rectangle openCVRectangleToDlib(const cv::Rect & r)
{
    return dlib::rectangle((long)r.tl().x, (long)r.tl().y, (long)r.br().x - 1, (long)r.br().y - 1);
}

#endif // UTILS_H

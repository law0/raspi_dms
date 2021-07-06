#include <opencv2/opencv.hpp>

#include <map>
#include <mutex>
/**
 * @brief timeMark
 * @param id
 * @param update, set to false if you don't want to update the internal time upon call of timeMark
 * @return For a same id given, time in seconds since boot the first time, time since last call the next times
 */
double inline timeMark(long id=0, bool update=true) {
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
long inline getUniqueId() {
    static long t = 0;
    return t++;
}

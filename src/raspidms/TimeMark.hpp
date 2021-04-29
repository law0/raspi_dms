#include <opencv2/opencv.hpp>


/**
 * @brief timeMark
 * @return time since boot the first time, time since last call of timeMark the next times
 */
double inline timeMark() {
    static double t = 0;
    double nt = static_cast<double>(cv::getTickCount());
    double ret = (nt - t)/cv::getTickFrequency();
    t = nt;
    return ret;
}

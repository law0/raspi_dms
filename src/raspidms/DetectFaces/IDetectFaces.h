#ifndef IDETECTFACES_H
#define IDETECTFACES_H

#include "IStage.h"

#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/core/utility.hpp>

#include <string>
#include <vector>

class IDetectFaces
{
public:
    IDetectFaces(const std::string & path,
                 const std::string & secondPath = std::string())
        : m_path(path), m_secondPath(secondPath) {}
    virtual ~IDetectFaces() {}

    /**
     * @brief operator ()
     * @param frame
     * @return pair of
     * - a vector of rectangles of faces (upleft, upright, w, h)
     * - a double : time in second it took for computing
     */
    virtual PointsList operator()(const cv::Mat& frame) = 0;

protected:
    const std::string m_path;
    const std::string m_secondPath;
};

#endif // IDETECTFACES_H

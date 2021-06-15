#include "DetectFacesResnetCaffe.h"

#include "TimeMark.hpp"

#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/core/utility.hpp>

DetectFacesResnetCaffe::DetectFacesResnetCaffe(const std::string & protoTxtPath, const std::string & caffeModelPath)
    : IDetectFaces(protoTxtPath, caffeModelPath)
{

}

std::pair<std::vector<cv::Rect>, double> DetectFacesResnetCaffe::operator()(cv::Mat frame) {
    static cv::dnn::Net net = cv::dnn::readNetFromCaffe(m_path, m_secondPath);
    double confThreshold = 0.5;
    timeMark();
    std::vector<cv::Rect> faces;

    cv::Mat frameCopy = frame.clone();
    cv::resize(frame, frameCopy, cv::Size(300, 300));

    cv::Mat blob;
    cv::dnn::blobFromImage(frameCopy, blob, 1.0, cv::Size(300, 300));
    net.setInput(blob);
    cv::Mat outs = net.forward();

    // Network produces output blob with a shape 1x1xNx7 where N is a number of
    // detections and an every detection is a vector of values
    // [batchId, classId, confidence, left, top, right, bottom]
    float* data = (float*)outs.data;
    for (size_t i = 0; i < outs.total(); i += 7)
    {
        float confidence = data[i + 2];
        if (confidence > confThreshold)
        {
            float left   = (float)data[i + 3];
            float top    = (float)data[i + 4];
            float right  = (float)data[i + 5];
            float bottom = (float)data[i + 6];
            float width  = right - left + 1;
            float height = bottom - top + 1;
            if (width <= 2 || height <= 2)
            {
                left   = (float)(data[i + 3] * frame.cols);
                top    = (float)(data[i + 4] * frame.rows);
                right  = (float)(data[i + 5] * frame.cols);
                bottom = (float)(data[i + 6] * frame.rows);
                width  = right - left + 1;
                height = bottom - top + 1;
            }
            faces.push_back(cv::Rect(left, top, width, height));
        }
    }

    return std::make_pair(faces, timeMark());
}

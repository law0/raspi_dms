#include "DetectFaces/DetectFacesMyYolo.h"

#include "Utils.h"

#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/core/utility.hpp>


DetectFacesMyYolo::DetectFacesMyYolo(const std::string & path) :
    IDetectFaces(path),
    m_net(cv::dnn::readNetFromONNX(path)),
    m_id(getUniqueId())
{

}

PointsList DetectFacesMyYolo::operator()(const cv::Mat & frame) {
    std::cout << "yolo" << std::endl;
    float confThreshold = 0.5;
    float classThreshold = 0.5;
    PointsList faces;

    if(frame.empty())
        return faces;

    cv::Mat frameCopy = frame.clone();
    cv::resize(frame, frameCopy, cv::Size(224, 224));

    const cv::Scalar std_dev = cv::Scalar(0.229, 0.224, 0.225);

    cv::Mat blob;
    cv::dnn::blobFromImage(frameCopy, blob,
                           1. / 255., //pixels values between 0 and 1
                           cv::Size(224, 224),
                           cv::Scalar(0.485, 0.456, 0.406), //substract mean
                           true //swap Red Green
                           );

    // Divide blob by std.
    cv::divide(blob, std_dev, blob);

    m_net.setInput(blob);
    cv::Mat outs = m_net.forward();

    // Network produces output blob with a shape 1x49x(1+4*5)
    // 49 Cells, and per cell : 1 class, 4 boxes, 1 confidence + 4 coords (x,y,w,h) per box.

    const float scale_x = 224. / frame.cols;
    const float scale_y = 224. / frame.rows;
    const int num_cells = 49; // 7 * 7 cells
    const int num_classes = 1;
    const int num_box_per_cells = 4;
    const int num_features_per_boxes = 5; //confidence, x, y, w, h
    float* data = (float*)outs.data;

    //Find the cell with max class
    float class_max = 0.0;
    size_t index_max = 0;
    size_t i_max = 0;
    for (size_t i = 0; i < num_cells; ++i)
    {
        size_t index = i * (num_classes + num_box_per_cells * num_features_per_boxes);
        float classe = data[index];
        if (classe > classThreshold && classe > class_max)
        {
            class_max = classe;
            index_max = index;
            i_max = i;
        }
    }

    int cell_x = floor(i_max % 7) * 32; // 224 / 7 = 32
    int cell_y = floor(i_max / 7) * 32;

    //in the max class cell, find the box with the max confidence
    float max_conf = 0.;
    size_t j_max = 0;
    for (size_t j = 0; j < num_box_per_cells * num_features_per_boxes; j += num_features_per_boxes)
    {
        float conf   = (float)data[index_max + j + 1];
        if (conf > confThreshold && conf > max_conf)
        {
            max_conf = conf;
            j_max = j;
        }
    }


    float width  = (float)data[index_max + j_max + 4];
    float height = (float)data[index_max + j_max + 5];

    width  = (float)(width * 224);
    height = (float)(height * 224);
    float left   = (float)(data[index_max + j_max + 2] * 32) + (float)cell_x - width / 2. ;
    float top    = (float)(data[index_max + j_max + 3] * 32) + (float)cell_y - height / 2. ;
    float right  = left + width;
    float bottom = top + height;

    /*std::cout << "Class " << class_max << ", conf " << max_conf << ", ("
              << left << ", "
              << top << ", "
              << width << ", "
              << height << ")"
              << std::endl;*/

    faces.push_back({cv::Point2f(left / scale_x, top / scale_y), cv::Point2f(right / scale_x, bottom / scale_y)});

    return faces;
}

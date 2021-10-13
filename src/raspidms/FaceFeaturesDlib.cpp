#include "FaceFeaturesDlib.h"

#include <dlib/opencv/cv_image.h>
#include <dlib/serialize.h>

#include <inttypes.h>
#include "Utils.h"

FaceFeaturesDlib::FaceFeaturesDlib(const std::string & path)
    : IFaceFeatures(path),
      m_shapePredictor(),
      m_id(getUniqueId())
{
    dlib::deserialize(m_path) >> m_shapePredictor;
    std::cout << "building face feature predictor (dlib) complete" << std::endl;
}

FaceFeaturesResult FaceFeaturesDlib::operator()(const cv::Mat & frame,
                                                std::vector<cv::Rect>& roi) {

    timeMark(m_id);
    cv::Mat frameCopy = frame.clone();

    dlib::array2d<dlib::bgr_pixel> dlibFrame;
    dlib::assign_image(dlibFrame, dlib::cv_image<dlib::bgr_pixel>(frameCopy));

    std::vector<std::vector<cv::Point2i>> ret;
    for (auto& region : roi) {
        std::vector<cv::Point2i> points;
        dlib::full_object_detection shape = m_shapePredictor(dlibFrame,
                                                             openCVRectangleToDlib(region));

        uint32_t num_parts = shape.num_parts();
        for(uint32_t i = 0; i < num_parts; ++i) {
            auto p = shape.part(i);
            points.push_back(cv::Point2i(p.x(), p.y()));
        }

        ret.push_back(points);
    }

    return std::make_pair(ret, timeMark(m_id));
}

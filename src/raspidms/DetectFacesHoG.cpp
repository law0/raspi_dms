#include "DetectFacesHoG.h"

#include "TimeMark.hpp"

#include <dlib/opencv/cv_image.h>

DetectFacesHoG::DetectFacesHoG(const std::string & path)
    : IDetectFaces(path),
      m_frontalFaceDetector(dlib::get_frontal_face_detector())
{

}

cv::Rect DetectFacesHoG::dlibRectangleToOpenCV(dlib::rectangle r)
{
    return cv::Rect(cv::Point2i(r.left(), r.top()), cv::Point2i(r.right() + 1, r.bottom() + 1));
}

dlib::rectangle DetectFacesHoG::openCVRectToDlib(cv::Rect r)
{
  return dlib::rectangle((long)r.tl().x, (long)r.tl().y, (long)r.br().x - 1, (long)r.br().y - 1);
}

std::pair<std::vector<cv::Rect>, double> DetectFacesHoG::operator()(cv::Mat frame) {
    timeMark();

    std::vector<cv::Rect> faces;

    cv::Mat frameCopy = frame.clone();
    cv::resize(frame, frameCopy, cv::Size(224, 224));

    const float scale_x = 224. / frame.cols;
    const float scale_y = 224. / frame.rows;

    dlib::array2d<dlib::bgr_pixel> dlibFrame;
    dlib::assign_image(dlibFrame, dlib::cv_image<dlib::bgr_pixel>(frameCopy));

    // Now tell the face detector to give us a list of bounding boxes
    // around all the faces it can find in the image.
    std::vector<dlib::rectangle> dets = m_frontalFaceDetector(dlibFrame);

    //to openCV rect and rescale
    std::for_each(dets.begin(), dets.end(), [&](dlib::rectangle r) {
        cv::Rect cvRect = DetectFacesHoG::dlibRectangleToOpenCV(r);
        cvRect.x /= scale_x;
        cvRect.y /= scale_y;
        cvRect.width /= scale_x;
        cvRect.height /= scale_y;
        faces.push_back(cvRect);
    });

    return std::make_pair(faces, timeMark());
}

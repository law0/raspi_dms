#include "DetectFaces/DetectFacesMediaPipe.h"

#include "Utils.h"

#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/core/utility.hpp>

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/optional_debug_tools.h"


#include <chrono>
#include <map>
#include <thread>
#include <vector>

/*
 * Part of this code is taken from :
 * https://github.com/google/mediapipe/blob/master/mediapipe/modules/face_detection/face_detection_short_range_common.pbtxt
 * https://github.com/google/mediapipe/blob/master/mediapipe/calculators/tflite/ssd_anchors_calculator.cc
 * https://github.com/google/mediapipe/blob/6abec128edd6d037e1a988605a59957c22f1e967/mediapipe/calculators/tensor/tensors_to_detections_calculator.cc
 * https://github.com/patlevin/face-detection-tflite/blob/fd03531abe5f8a7bd5fbade9ab5dd128caf2072d/fdlite/face_detection.py#L288
 *
 * Mostly for anchors generation and boxes conversions
 *
 * Also this only work with model face_detection_short_range.tflite, which comes from :
 * https://github.com/google/mediapipe/raw/v0.8.9/mediapipe/modules/face_detection/face_detection_short_range.tflite
 */

static const std::map<std::string, double> kInputParameters = {
    {"num_layers", 4},
    {"min_scale", 0.1484375},
    {"max_scale", 0.75},
    {"input_size_height", 128},
    {"input_size_width", 128},
    {"anchor_offset_x", 0.5},
    {"anchor_offset_y", 0.5},
    {"aspect_ratios", 1.0},
    //{"fixed_anchor_size", 1.0},
    {"interpolated_scale_aspect_ratio", 1.0},
};

static const std::vector<int> kInputStrides = {8, 16, 16, 16};

static const std::map<std::string, double> kOutputParameters = {
    {"num_classes", 1},
    {"num_boxes", 896},
    {"num_coords", 16},
    {"box_coord_offset", 0},
    {"keypoint_coord_offset", 4},
    {"num_keypoints", 6},
    {"num_values_per_keypoint", 2},
    {"sigmoid_score", 1.0},
    {"score_clipping_thresh", 100.0},
    {"reverse_output_order", 1.0},
    {"x_scale", 128.0},
    {"y_scale", 128.0},
    {"h_scale", 128.0},
    {"w_scale", 128.0},
    {"min_score_thresh", 0.5},
};

static tflite::FlatBufferModel& getModel(const std::string & path) {
    static std::unique_ptr<tflite::FlatBufferModel> flatBufferModel(nullptr);
    if (!flatBufferModel)
        flatBufferModel = tflite::FlatBufferModel::BuildFromFile(path.c_str(), nullptr);

    return *flatBufferModel;
}

static tflite::ops::builtin::BuiltinOpResolver& getResolver() {
    static tflite::ops::builtin::BuiltinOpResolver resolver;
    return resolver;
}

DetectFacesMediaPipe::DetectFacesMediaPipe(const std::string & modelPath)
    : IDetectFaces(modelPath),
      m_id(getUniqueId()),
      m_interpreter(nullptr),
      m_anchors()
{
    generate_anchors();
    std::cout << "m_anchors size: " << m_anchors.size() << std::endl;
    if (m_anchors.size() != static_cast<int>(kOutputParameters.at("num_boxes"))) {
        std::cout << __FUNCTION__<< ":" << __LINE__
                  << "Error: num of anchors (" << m_anchors.size() << ") should be "
                  << static_cast<int>(kOutputParameters.at("num_boxes")) << std::endl;
        abort();
    }
}

void DetectFacesMediaPipe::printModelIOTensorsInfo() {
    if (!m_interpreter) {
        std::cout << "Null interpreter" << std::endl;
        return;
    }

    int num(0);
    int i(0);
    TfLiteIntArray* dims(nullptr);

    num = m_interpreter->inputs().size();
    std::cout << "Number of inputs tensor: " << num << std::endl;
    for (i = 0; i < num; ++i) {
        auto input = m_interpreter->input_tensor(i);
        if (!input)
            continue;
        std::cout << "input_tensor("<< i << ").bytes = " << input->bytes << std::endl
                  << "input_tensor("<< i << ").type = " << input->type << std::endl;
        dims = input->dims;
        if (dims) {
            std::cout << "input_tensor("<< i << ").dims = [";
            for (int i = 0; i < dims->size; ++i) {
                std::cout << dims->data[i] << ", ";
            }
            std::cout << "]" << std::endl;
        }
    }

    num = m_interpreter->outputs().size();
    std::cout << "Number of outputs tensor: " << num << std::endl;
    for (i = 0; i < num; ++i) {
        auto output = m_interpreter->output_tensor(i);
        if (!output)
            continue;
        std::cout << "output_tensor("<< i << ").bytes = " << output->bytes << std::endl
                  << "output_tensor("<< i << ").type = " << output->type << std::endl;
        dims = output->dims;
        if (dims) {
            std::cout << "output_tensor("<< i << ").dims = [";
            for (int i = 0; i < dims->size; ++i) {
                std::cout << dims->data[i] << ", ";
            }
            std::cout << "]" << std::endl;
        }
    }
}

PointsList DetectFacesMediaPipe::operator()(const cv::Mat & frame) {
    //std::cout << "Tflite" << std::endl;
    PointsList faces;

    if(frame.empty()) {
        std::cout << "Empty frame !" << std::endl;
        return faces;
    }

    if (!m_interpreter) {
        auto& model = getModel(m_path);
        auto& resolver = getResolver();

        tflite::InterpreterBuilder builder(model, resolver);
        if (builder(&m_interpreter) != kTfLiteOk) {
            std::cout << "Error building interpreter" << std::endl;
            m_interpreter.reset();
            return faces;
        } else {
            printModelIOTensorsInfo();
        }

        if (m_interpreter->AllocateTensors() != kTfLiteOk) {
            m_interpreter.reset();
            std::cout << "Error allocating tensors" << std::endl;
            return faces;
        }
    }

    //std::cout << "frame (chan,c,r,t): " << frame.channels() << " " << frame.cols << " " << frame.rows << " " << frame.type() << std::endl;
    cv::Mat frameCopy = frame.clone();
    cv::resize(frame, frameCopy, cv::Size(kInputParameters.at("input_size_width"), kInputParameters.at("input_size_height"))); //mediapipe face_detection_short_range input size is 128x128
    cv::cvtColor(frameCopy, frameCopy, cv::COLOR_BGR2RGB);
    frameCopy.convertTo(frameCopy, CV_32FC3, 1.0 / 127, -1.0); //see mediapipe Face Detection model card

    /*std::cout << frameCopy.channels() << " " << frameCopy.cols << " " << frameCopy.rows << " "
              << frameCopy.type() << " " << frameCopy.total() << std::endl;*/



    float* data = reinterpret_cast<float*>(frameCopy.data);
    float* input_f32 = m_interpreter->typed_input_tensor<float>(0);
    // frameCopy.total() * frameCopy.channels() = (128 * 128) * 3 = num of floats
    for (size_t i = 0; i < frameCopy.total() * frameCopy.channels(); i++) {
        input_f32[i] = data[i];
    }
    m_interpreter->Invoke();

    //mediapipe face_detection_short_range as two output tensors of dims :
    //[1, 896, 16] for output(0)
    //[1, 896, 1]  for output(1)
    //output(0) last dim is contains a bounding box (xmin, ymin, width, height) and 6 (x,y) key points
    //(right eye, left eye, nose tip, mouth center, right ear tragion, and left ear tragion)
    //(in that order, all between [0, 1] normalized by image dimension)
    // see https://google.github.io/mediapipe/solutions/face_detection.html

    float* output_f32_0 = m_interpreter->typed_output_tensor<float>(0);
    float* output_f32_1 = m_interpreter->typed_output_tensor<float>(1);

    if (!output_f32_0 || !output_f32_1) {
        std::cout << "Error output tensors" << std::endl;
        return faces;
    }

    const float x_scale = kOutputParameters.at("x_scale");
    const float y_scale = kOutputParameters.at("y_scale");
    const float h_scale = kOutputParameters.at("h_scale");
    const float w_scale = kOutputParameters.at("w_scale");

    const float threshold = kOutputParameters.at("min_score_thresh");

    const int num_kp = kOutputParameters.at("num_keypoints");
    const int num_val_per_kp = kOutputParameters.at("num_values_per_keypoint");

    const bool reverse_output_order = kOutputParameters.at("reverse_output_order") > 0.f;

    const float bounding_box_scale_up = 2.f;

    for (size_t i = 0; i < static_cast<int>(kOutputParameters.at("num_boxes")); ++i) {
        if (output_f32_1[i] > threshold) {
            int box_offset = static_cast<int>(i * kOutputParameters.at("num_coords") + kOutputParameters.at("box_coord_offset"));

            float y_center = output_f32_0[box_offset];
            float x_center = output_f32_0[box_offset + 1];
            float h = output_f32_0[box_offset + 2];
            float w = output_f32_0[box_offset + 3];
            if (reverse_output_order) {
              x_center = output_f32_0[box_offset];
              y_center = output_f32_0[box_offset + 1];
              w = output_f32_0[box_offset + 2];
              h = output_f32_0[box_offset + 3];
            }

            float h_anchor = 1.0; //m_anchors[i][2] if not fixed_anchors_size
            float w_anchor = 1.0; //m_anchors[i][3] if not fixed_anchors_size

            x_center = x_center / x_scale * h_anchor + m_anchors[i][0];
            y_center = y_center / y_scale * w_anchor + m_anchors[i][1];
            h = h / h_scale * h_anchor;
            w = w / w_scale * w_anchor;

            const float left = (x_center - w / 2.f) * frame.cols;
            const float top = (y_center - h / 2.f * bounding_box_scale_up) * frame.rows;
            const float right = (x_center + w / 2.f) * frame.cols;
            const float bottom = (y_center + h / 2.f * bounding_box_scale_up) * frame.rows;

            const float width = w * frame.cols;
            const float height = h * frame.rows;

            if (width >= 0 && height >= 0 && left >= 0 && top >= 0) {
                std::vector<cv::Point2f> points;

                //head box
                points.push_back(cv::Point2f(left, top));
                points.push_back(cv::Point2f(right, bottom));

                //eyes, nose, mouth, ears, points
                // Keypoints are unused for now
                int keypoint_index = 0;
                float keypoint_x = 0.f;
                float keypoint_y = 0.f;
                for (int j = 0; j < num_kp * num_val_per_kp ; j+= num_val_per_kp) {
                    keypoint_index = box_offset + kOutputParameters.at("keypoint_coord_offset") + j;
                    keypoint_y = output_f32_0[keypoint_index];
                    keypoint_x = output_f32_0[keypoint_index + 1];
                    if (reverse_output_order) {
                        keypoint_x = output_f32_0[keypoint_index];
                        keypoint_y = output_f32_0[keypoint_index + 1];
                    }

                    keypoint_x = keypoint_x / x_scale * w_anchor + m_anchors[i][0] * frame.cols;
                    keypoint_y = keypoint_y / y_scale * h_anchor + m_anchors[i][1] * frame.rows;

                    points.push_back(cv::Point2f(keypoint_x, keypoint_y));
                }

                faces.push_back(points);
            }
        }
    }

    if (faces.empty())
        return faces;

    // Eliminate excessive detections
    PointsList final_faces;
    final_faces.push_back(faces.back());
    faces.pop_back();

     if (faces.size() > 0) {
        // O(n^2), but there shouldn't be much faces anyway (~3 faces max)
        for (int i = 0; i < faces.size(); ++i) {
            for (int j = 0; j < final_faces.size(); ++j) {
                auto a = cv::Rect(faces[i][0], faces[i][1]);
                auto b = cv::Rect(final_faces[j][0], final_faces[j][1]);

                // If faces overlap much with another
                if (iou_score(a, b) > 0.6) {
                    // Then they detect the same face, take the biggest bounding box
                    if (a.area() > b.area()) {
                        final_faces[j] = faces[i];
                    } else {
                        continue;
                    }
                } else {
                    // If no overlap, it is a new face
                    final_faces.push_back(faces[i]);
                }
            }
        }
    }

    //std::cout << __FUNCTION__ << " final_faces.size() = " << final_faces.size() << std::endl;
    /*for (auto vec : final_faces) {
        std::cout << "head (" << vec[0] << ", " << vec[1] << ")" << std::endl
                  << "r eye, l eye, nose, mouth, r ear, l ear ("
                  << vec[2] << ", " << vec[3] << ", " << vec[4] << ", " << vec[5]
                  << vec[6] << ", " << vec[7] << ")" << std::endl;
    }*/

    return final_faces;
}

void DetectFacesMediaPipe::generate_anchors() {
    const int num_layers = kInputParameters.at("num_layers");
    const bool interpolated_scale_aspect_ratio = kInputParameters.at("interpolated_scale_aspect_ratio") > 0;
    const int width = kInputParameters.at("input_size_width");
    const int height = kInputParameters.at("input_size_height");
    const float anchor_offset_x = kInputParameters.at("anchor_offset_x");
    const float anchor_offset_y = kInputParameters.at("anchor_offset_y");
    int layer_id = 0;
    int last_same_stride_layer = 0;
    int repeats = 0;
    int stride = 0;
    int feature_map_width = 0;
    int feature_map_height = 0;
    float x_center = 0.;
    float y_center = 0.;

    while (layer_id < num_layers) {
        last_same_stride_layer = layer_id;
        repeats = 0;
        while (last_same_stride_layer < num_layers
               && kInputStrides[last_same_stride_layer] == kInputStrides[layer_id]) {
            last_same_stride_layer++;
            // twice if interpolated_scale_aspect_ratio
            if (interpolated_scale_aspect_ratio > 0)
                repeats++;
            repeats++;
        }

        stride = kInputStrides[layer_id];
        feature_map_width = width / stride;
        feature_map_height = height / stride;
        for (int y = 0; y < feature_map_height; ++y) {
            y_center = (y + anchor_offset_y) / feature_map_height;
            for (int x = 0; x < feature_map_width; ++x) {
                x_center = (x + anchor_offset_x) / feature_map_width;
                for (int i = 0; i < repeats; ++i) {
                    m_anchors.push_back({x_center, y_center});
                }
            }
        }
        layer_id = last_same_stride_layer;
    };
}

float DetectFacesMediaPipe::iou_score(const cv::Rect& a, const cv::Rect& b) const {
    float intersection_area = static_cast<float>((a & b).area());
    float union_area = static_cast<float>(a.area() + b.area() - intersection_area);
    return intersection_area / union_area;
}

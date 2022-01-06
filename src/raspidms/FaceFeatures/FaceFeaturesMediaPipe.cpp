#include "FaceFeatures/FaceFeaturesMediaPipe.h"

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

#include <inttypes.h>
#include <map>
#include <thread>
#include <vector>


static const std::map<std::string, double> kInputParameters = {
    {"input_size_height", 192},
    {"input_size_width", 192},
    // model is optimized for 25% padding,
    // but 10% allows better precision at image boundaries and still works well
    {"roi_scale", 1.1},
};

static const std::map<std::string, double> kOutputParameters = {
    {"num_landmarks", 468},
    {"detection_threshold", 0.5},
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

FaceFeaturesMediaPipe::FaceFeaturesMediaPipe(const std::string & path)
    : IFaceFeatures(path),
      m_interpreter(nullptr),
      m_id(getUniqueId())
{

}

void FaceFeaturesMediaPipe::printModelIOTensorsInfo() {
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

PointsList FaceFeaturesMediaPipe::operator()(const cv::Mat & frame,
                                                std::vector<cv::Rect>& roi) {
    PointsList ret;
    if (frame.empty()) {
        return ret;
    }

    if (!m_interpreter) {
        auto& model = getModel(m_path);
        auto& resolver = getResolver();

        tflite::InterpreterBuilder builder(model, resolver);
        if (builder(&m_interpreter) != kTfLiteOk) {
            std::cout << "Error building interpreter" << std::endl;
            m_interpreter.reset();
            return ret;
        } else {
            printModelIOTensorsInfo();
        }

        if (m_interpreter->AllocateTensors() != kTfLiteOk) {
            m_interpreter.reset();
            std::cout << "Error allocating tensors" << std::endl;
            return ret;
        }
    }

    cv::Mat frameCopy = frame.clone();
    cv::cvtColor(frameCopy, frameCopy, cv::COLOR_BGR2RGB);

    const float roi_scale = kInputParameters.at("roi_scale");
    const int num_landmarks = kOutputParameters.at("num_landmarks");
    const float detection_threshold = kOutputParameters.at("detection_threshold");

    for (auto region : roi) {

        // scale up roi
        float new_left   = std::max(0.f,                   roi_scale * region.tl().x + (1.f - roi_scale) * region.br().x);
        float new_right  = std::min((float)frameCopy.cols, roi_scale * region.br().x + (1.f - roi_scale) * region.tl().x);
        float new_top    = std::max(0.f,                   roi_scale * region.tl().y + (1.f - roi_scale) * region.br().y);
        float new_bottom = std::min((float)frameCopy.rows, roi_scale * region.br().y + (1.f - roi_scale) * region.tl().y);

        region = cv::Rect(cv::Point2f(new_left, new_top), cv::Point2f(new_right, new_bottom));
        float roi2image_width_scale = (float)region.width / kInputParameters.at("input_size_width");
        float roi2image_height_scale = (float)region.height / kInputParameters.at("input_size_height");

        cv::Mat croppedFrame = frameCopy(region);
        cv::resize(croppedFrame, croppedFrame, cv::Size(kInputParameters.at("input_size_width"), kInputParameters.at("input_size_height"))); //mediapipe face_detection_short_range input size is 128x128
        croppedFrame.convertTo(croppedFrame, CV_32FC3, 1.0 / 127, -1.0); //see mediapipe Face Detection model card*/

        float* data = reinterpret_cast<float*>(croppedFrame.data);
        float* input_f32 = m_interpreter->typed_input_tensor<float>(0);
        // croppedFrame.total() * croppedFrame.channels() = (192 * 192) * 3 = num of floats
        for (size_t i = 0; i < croppedFrame.total() * croppedFrame.channels(); i++) {
            input_f32[i] = data[i];
        }

        m_interpreter->Invoke();

        float* output_f32_0 = m_interpreter->typed_output_tensor<float>(0);
        float* output_f32_1 = m_interpreter->typed_output_tensor<float>(1);

        if (!output_f32_0 || !output_f32_1) {
            std::cout << "Error output tensors" << std::endl;
            return ret;
        }

        // output_f32_1 is of dim [1, 1, 1, 1] so just one float
        if (*output_f32_1 > detection_threshold) {
            ret.emplace_back(num_landmarks, cv::Point2f());
            auto& points = ret.back();
            for (int i = 0; i < num_landmarks; ++i) {
                // i * 3 because each landmarks is (x, y, z)
                // z is discarded here
                points[i] = cv::Point2f(output_f32_0[i * 3] * roi2image_width_scale + region.x,
                        output_f32_0[i * 3 + 1] * roi2image_height_scale + region.y);
            }
        }
    }

    return ret;
}

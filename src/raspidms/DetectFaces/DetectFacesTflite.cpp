#include "DetectFaces/DetectFacesTflite.h"

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

DetectFacesTflite::DetectFacesTflite(const std::string & modelPath)
    : IDetectFaces(modelPath),
      m_id(getUniqueId()),
      m_interpreter(nullptr)
{

}


DetectedFacesResult DetectFacesTflite::operator()(const cv::Mat & frame) {
    std::cout << __FUNCTION__ << " ";
    double confThreshold = 0.5;
    timeMark(m_id);
    std::vector<cv::Rect> faces;

    if(frame.empty())
        return std::make_pair(faces, 0.0);

    if (!m_interpreter) {
        auto& model = getModel(m_path);
        auto& resolver = getResolver();

        tflite::InterpreterBuilder builder(model, resolver);
        if (builder(&m_interpreter) != kTfLiteOk) {
            m_interpreter.reset();
            return std::make_pair(faces, 0.0);
        }
    }

    std::cout << frame.channels() << " " << frame.cols << " " << frame.rows << " " << frame.type() << std::endl;
    cv::Mat frameCopy = frame.clone();
    cv::resize(frame, frameCopy, cv::Size(416, 416));
    frameCopy.convertTo(frameCopy, CV_32FC3, 1.0 / 255);

    std::cout << frameCopy.channels() << " " << frameCopy.cols << " " << frameCopy.rows << " "
              << frameCopy.type() << " " << frameCopy.total() << std::endl;

    auto input = m_interpreter->input_tensor(0);
    std::cout << __FUNCTION__ << "inputs().size() = " << m_interpreter->inputs().size() << std::endl
              << "input_tensor(0).bytes = " << input->bytes << std::endl
              << "input_tensor(0).type = " << input->type << std::endl;
    TfLiteIntArray* dims = input->dims;
    if (dims) {
        std::cout << "input_tensor(0).dims = [";
        for (int i = 0; i < dims->size; ++i) {
            std::cout << dims->data[i] << ", ";
        }
        std::cout << "]" << std::endl;
    }

    if (m_interpreter->AllocateTensors() != kTfLiteOk) {
        m_interpreter.reset();
        return std::make_pair(faces, 0.0);
    }

    float* data = reinterpret_cast<float*>(frameCopy.data);
    float* input_f32 = m_interpreter->typed_input_tensor<float>(0);
    // frameCopy.total() * frameCopy.channels() = (416 * 416) * 3 = num of floats
    for (size_t i = 0; i < frameCopy.total() * frameCopy.channels(); i++) {
        input_f32[i] = data[i];
    }
    m_interpreter->Invoke();

    auto output = m_interpreter->output_tensor(0);
    std::cout << __FUNCTION__ << "outputs().size() = " << m_interpreter->outputs().size() << std::endl
              << "output_tensor(0).bytes = " << output->bytes << std::endl
              << "output_tensor(0).type = " << output->type << std::endl;
    dims = output->dims;
    if (dims) {
        std::cout << "output_tensor(0).dims = [";
        for (int i = 0; i < dims->size; ++i) {
            std::cout << dims->data[i] << ", ";
        }
        std::cout << "]" << std::endl;
    }

    return std::make_pair(faces, timeMark(m_id));
}

#include "remote_message.grpc.pb.h"

#include <iostream>
#include <fstream>
#include <opencv4/opencv2/opencv.hpp>
#include "yolov4.h"

using remote::ProtoMat;
using namespace std;

YOLOv4::YOLOv4() 
{
    string yoloVersion("v4-tiny");
    string yoloFolderPath("../yolov4/");
    
    const string yoloPath(yoloFolderPath + "yolo" + yoloVersion);
    auto net = LoadNet(yoloPath + ".cfg", yoloPath + ".weights", false); // if use cuda : true
    
    _model = std::make_unique<cv::dnn::DetectionModel>(net);
    _model->setInputParams(1./255, cv::Size(416, 416), cv::Scalar(), true);   
    
    _classList = LoadClassList(yoloFolderPath + "classes.txt");
    
    _colors = {cv::Scalar(255, 255, 0), cv::Scalar(0, 255, 0), cv::Scalar(0, 255, 255), cv::Scalar(255, 0, 0)};
}

auto YOLOv4::LoadClassList(const string& path) -> vector<std::string>
{
    std::vector<std::string> result;
    std::string line;
    std::ifstream ifs(path);
    
    while (getline(ifs, line))
        result.push_back(line);

    ifs.close();
    
    return result;
}

auto YOLOv4::LoadNet(const string& cfgPath, const string& weightsPath, bool is_cuda) -> cv::dnn::Net
{
    auto result = cv::dnn::readNetFromDarknet(cfgPath, weightsPath);

    if (is_cuda) 
    {
        std::cout << "Attempty to use CUDA\n";
        result.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        result.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA_FP16);
    } 
    else 
    {
        std::cout << "Running on CPU\n";
        result.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        result.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    }

    return result;
}

auto YOLOv4::DetectObject(cv::Mat frame) -> cv::Mat
{       
    std::vector<int> classIds;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;
    _model->detect(frame, classIds, confidences, boxes, .2, .4);

    int detections = classIds.size();

    for (auto i = 0; i < detections; ++i) 
    {
        auto box = boxes[i];
        auto classId = classIds[i];
        const auto color = _colors[classId % _colors.size()];

        cv::rectangle(frame, box, color, 1);
        cv::rectangle(frame, cv::Point(box.x, box.y - 20), cv::Point(box.x + box.width, box.y), color, cv::FILLED);

        cv::putText(frame, _classList[classId].c_str(), cv::Point(box.x, box.y - 5), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 0), 2);
    }

    return frame;
}
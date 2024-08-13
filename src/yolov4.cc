#include <iostream>
#include <fstream>
#include <opencv4/opencv2/opencv.hpp>

#include "remote_message.grpc.pb.h"
#include "yolov4.h"

using remote::ProtoMat;
using remote::YoloData;
using remote::Object;
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

auto YOLOv4::DetectYOLO(cv::Mat frame) -> YoloData
{   
    YoloData objects;
    std::vector<int> classIds;
    std::vector<float> confidences;
    std::vector<cv::Rect> rects;

    _model->detect(frame, classIds, confidences, rects, .2, .4);

    int detections = classIds.size();

    for (auto i = 0; i < detections; ++i) 
    {
        Object* obj = objects.add_objects();
        
        obj->set_classname(_classList[classIds[i]]);
        obj->set_classid(classIds[i]);
        obj->set_confidence(confidences[i]);
        obj->set_tl_x(rects[i].x);
        obj->set_tl_y(rects[i].y);
        obj->set_width(rects[i].width);
        obj->set_height(rects[i].height);
    }   

    return objects;
}
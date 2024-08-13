#include <iostream>
#include <fstream>
#include <opencv4/opencv2/opencv.hpp>

#include "remote_message.grpc.pb.h"
#include "yolov4.h"

using remote::ProtoMat;
using remote::YoloData;
using remote::Object;

using namespace std;
using namespace cv;

YOLOv4::YOLOv4() 
{
    string yoloVersion("v4-tiny");
    string yoloFolderPath("../yolov4/");
    
    const string yoloPath(yoloFolderPath + "yolo" + yoloVersion);
    auto net = LoadNet(yoloPath + ".cfg", yoloPath + ".weights", false); // if use cuda : true
    
    _model = make_unique<dnn::DetectionModel>(net);
    _model->setInputParams(1./255, Size(416, 416), Scalar(), true);   
    
    _classList = LoadClassList(yoloFolderPath + "classes.txt");
    
    _colors = {Scalar(255, 255, 0), Scalar(0, 255, 0), Scalar(0, 255, 255), Scalar(255, 0, 0)};
}

auto YOLOv4::LoadClassList(const string& path) -> vector<string>
{
    vector<string> result;
    string line;
    ifstream ifs(path);
    
    while (getline(ifs, line))
        result.push_back(line);

    ifs.close();
    
    return result;
}

auto YOLOv4::LoadNet(const string& cfgPath, const string& weightsPath, bool is_cuda) -> dnn::Net
{
    auto result = dnn::readNetFromDarknet(cfgPath, weightsPath);

    if (is_cuda) 
    {
        cout << "Attempty to use CUDA\n";
        result.setPreferableBackend(dnn::DNN_BACKEND_CUDA);
        result.setPreferableTarget(dnn::DNN_TARGET_CUDA_FP16);
    } 
    else 
    {
        cout << "Running on CPU\n";
        result.setPreferableBackend(dnn::DNN_BACKEND_OPENCV);
        result.setPreferableTarget(dnn::DNN_TARGET_CPU);
    }

    return result;
}

auto YOLOv4::DetectYOLO(Mat frame) -> YoloData
{   
    YoloData objects;
    vector<int> classIds;
    vector<float> confidences;
    vector<Rect> rects;

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
#ifndef YOLOV4_H
#define YOLOV4_H 

/*
[Reference link] https://github.com/improvess/yOLOv4-opencv-cpp-python

The contents of YOLOv4 class are references from above link
*/

class YOLOv4
{
public:
    YOLOv4();
    auto LoadClassList(const std::string& path) -> std::vector<std::string>;
    auto LoadNet(const std::string& cfgPath, const std::string& weightsPath, bool is_cuda) -> cv::dnn::Net;
    auto DetectObject(cv::Mat frame) -> remote::DetectedList;
    
private:
    std::unique_ptr<cv::dnn::DetectionModel> _model;
    std::vector<std::string> _classList;
    std::vector<cv::Scalar> _colors;
};

#endif
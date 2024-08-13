#ifndef VIDEO_H
#define VIDEO_H 

class VideoMaker
{
public:
    VideoMaker(cv::VideoCapture);
    void PlayVideo(void);
    void SaveVideoTo(const std::string&);
    void MergeYoloDataToVideo();
    void PushBack(const cv::Mat&);
    void PushBack(remote::YoloData);
    int GetTotalCount(void);

private:
    std::vector<remote::YoloData> _yoloDataList;
    std::vector<cv::Scalar> _colors;
    std::vector<cv::Mat> _images;
    cv::VideoCapture _cap;

    int _width;
    int _height;
    int _fps;
    int _count;
};

#endif
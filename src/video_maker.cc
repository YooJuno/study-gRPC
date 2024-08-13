#include <iostream>
#include <fstream>
#include <opencv4/opencv2/opencv.hpp>

#include "remote_message.grpc.pb.h"
#include "video_maker.h"

using remote::YoloData;
using remote::Object;

using namespace std;
using namespace cv;

VideoMaker::VideoMaker(VideoCapture cap)
    : _cap(cap)
{   
    _fps        = cap.get(CAP_PROP_FPS);
    _width      = cap.get(CAP_PROP_FRAME_WIDTH);
    _height     = cap.get(CAP_PROP_FRAME_HEIGHT);
    _count      = cap.get(CAP_PROP_FRAME_COUNT);
    _colors = {Scalar(255, 255, 0), Scalar(0, 255, 0), Scalar(0, 255, 255), Scalar(255, 0, 0), Scalar(100,255,100)};
}

void VideoMaker::PlayVideo()
{
    cout << "Video Play\n";

    for(const auto& image : _images)
    {
        imshow("video", image);
        if (waitKey(1000/_fps) == 27 /* ESC */) 
            break;
    }

    destroyWindow("video");
}

void VideoMaker::SaveVideoTo(const string& path)
{
    cout << "Saving video " + path << endl;
    VideoWriter videowriter(path, VideoWriter::fourcc('X', 'V', 'I', 'D'), _fps , Size(_width, _height), true);

    if (!videowriter.isOpened())
        return;

    for(const auto& image : _images)
        videowriter << image;

    cout << "Complete!" << endl;
}

void VideoMaker::MergeYoloDataToVideo()
{
    for(auto& yolo : _yoloDataList)
    {
        for (int i = 0; i < yolo.objects_size(); i++)
        {
            const auto color = _colors[yolo.objects(i).classid() % _colors.size()];
            Rect objectBox(yolo.objects(i).tl_x(), yolo.objects(i).tl_y(), yolo.objects(i).width(), yolo.objects(i).height());
            Rect textBox(yolo.objects(i).tl_x(), yolo.objects(i).tl_y() - 20, yolo.objects(i).width(), 20);
            rectangle(_images[yolo.seq()], objectBox, color, 2);
            rectangle(_images[yolo.seq()], textBox, color, FILLED);
            putText(_images[yolo.seq()], yolo.objects(i).classname(), Point(yolo.objects(i).tl_x(), yolo.objects(i).tl_y() - 5), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0));
        }
    }
}

void VideoMaker::PushBack(const Mat& image)
{
    _images.push_back(image.clone());
}

void VideoMaker::PushBack(YoloData yolo)
{
    _yoloDataList.push_back(yolo);
}

int VideoMaker::GetTotalCount()
{
    return _count;
}


#ifndef MEDIA_H
#define MEDIA_H 

class MediaHandler
{
public:
    auto ConvertProtoMatToMat(const remote::ProtoMat&) -> cv::Mat;
    auto ConvertMatToProtoMat(cv::Mat) -> remote::ProtoMat;
};

#endif
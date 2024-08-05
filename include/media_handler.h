#ifndef MEDIA_H
#define MEDIA_H 

class MediaHandler
{
public:
    auto ConvertProtomatToMat(const remote::ProtoMat& protomat) -> cv::Mat;
    auto ConvertMatToProtomat(const cv::Mat& image) -> remote::ProtoMat;
};

#endif
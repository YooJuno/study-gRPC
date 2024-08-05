#ifndef MEDIA_H
#define MEDIA_H 

class MediaHandler
{
public:
    cv::Mat ConvertProtomatToMat(const remote::ProtoMat& protomat);
    remote::ProtoMat ConvertMatToProtomat(const cv::Mat& image);
};

#endif
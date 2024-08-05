#include <iostream>
#include <opencv4/opencv2/opencv.hpp>
#include "remote_message.grpc.pb.h"
#include "media_handler.h"

using remote::ProtoMat;
using namespace std;

#define COLOR 3
#define GRAY 1

auto MediaHandler::ConvertProtomatToMat(const ProtoMat& protomat) -> cv::Mat
{
    cv::Mat img = cv::Mat(cv::Size(protomat.width(), protomat.height()), protomat.type());
    string serializedMatrix(protomat.buffer());
    int idx = 0;

    for (auto row=0 ; row<img.rows ; row++)
    {
        uchar* pointer_row = img.ptr<uchar>(row); 

        for (auto col=0 ; col<img.cols ; col++)
        {  
            if (protomat.channels() == GRAY)
            {   
                img.at<uchar>(row, col) = serializedMatrix[idx++];
            }
            else
            {
                pointer_row[col * 3 + 0] = serializedMatrix[idx++]; 
                pointer_row[col * 3 + 1] = serializedMatrix[idx++]; 
                pointer_row[col * 3 + 2] = serializedMatrix[idx++]; 
            }
        }
    }

    return img;
}

auto MediaHandler::ConvertMatToProtomat(const cv::Mat& image) -> ProtoMat
{
    ProtoMat result;
    string buffer("");
    
    result.set_width(image.size().width);
    result.set_height(image.size().height);
    result.set_channels(image.channels());
    result.set_type(image.type());
    result.set_seq(result.seq() + 1);
    
    for (auto i=0 ; i<image.size().height ; i++)
    {
        for (auto j=0 ; j<image.size().width ; j++)
        {
            if (image.channels() == GRAY)
            {       
                buffer += static_cast<uchar>(image.at<uchar>(i, j));
            }
            else if (image.channels() == COLOR)
            {
                const cv::Vec3b& pixel = image.at<cv::Vec3b>(i, j);

                buffer += static_cast<uchar>(pixel[0]); // B
                buffer += static_cast<uchar>(pixel[1]); // G
                buffer += static_cast<uchar>(pixel[2]); // R
            }
        }
    }

    result.set_buffer(buffer); 
    
    return result;
}
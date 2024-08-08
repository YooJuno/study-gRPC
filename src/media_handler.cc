#include <iostream>
#include <opencv4/opencv2/opencv.hpp>
#include "remote_message.grpc.pb.h"
#include "media_handler.h"

using remote::ProtoMat;
using namespace std;

#define COLOR 3
#define GRAY 1

auto MediaHandler::ConvertProtoMatToMat(const ProtoMat& protoMat) -> cv::Mat
{
    cv::Mat image = cv::Mat(cv::Size(protoMat.width(), protoMat.height()), protoMat.type());
    string serializedMatrix(protoMat.buffer());
    int idx = 0;

    for (auto row=0 ; row<image.rows ; row++)
    {
        uchar* pointer_row = image.ptr<uchar>(row); 

        for (auto col=0 ; col<image.cols ; col++)
        {  
            if (protoMat.channels() == GRAY)
            {   
                pointer_row[col] = serializedMatrix[idx++];
            }
            else
            {
                pointer_row[col * 3 + 0] = serializedMatrix[idx++]; 
                pointer_row[col * 3 + 1] = serializedMatrix[idx++]; 
                pointer_row[col * 3 + 2] = serializedMatrix[idx++]; 
            }
        }
    }

    return image;
}

auto MediaHandler::ConvertMatToProtoMat(cv::Mat image) -> ProtoMat
{
    ProtoMat result;
    string buffer("");
    
    result.set_width(image.size().width);
    result.set_height(image.size().height);
    result.set_channels(image.channels());
    result.set_type(image.type());
    result.set_seq(result.seq() + 1);
    
    for (auto row=0 ; row<image.rows ; row++)
    {
        uchar* pointer_row = image.ptr<uchar>(row);

        for (auto col=0 ; col<image.cols ; col++)
        {
            if (image.channels() == GRAY)
            {       
                buffer += static_cast<uchar>(pointer_row[col]);
            }
            else if (image.channels() == COLOR)
            {
                buffer += static_cast<uchar>(pointer_row[col * 3 + 0]); // B
                buffer += static_cast<uchar>(pointer_row[col * 3 + 1]); // G
                buffer += static_cast<uchar>(pointer_row[col * 3 + 2]); // R
            }
        }
    }

    result.set_buffer(buffer); 
    
    return result;
}
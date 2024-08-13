#include <iostream>
#include <opencv4/opencv2/opencv.hpp>
#include "remote_message.grpc.pb.h"
#include "image_handler.h"

using remote::ProtoMat;
using namespace std;

#define COLOR 3
#define GRAY 1

auto ImageHandler::ConvertProtoMatToMat(const ProtoMat& protoMat) -> cv::Mat
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

auto ImageHandler::ConvertMatToProtoMat(cv::Mat image) -> ProtoMat
{
    ProtoMat output;
    string buffer("");
    
    output.set_width(image.size().width);
    output.set_height(image.size().height);
    output.set_channels(image.channels());
    output.set_type(image.type());
    output.set_seq(output.seq() + 1);
    
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

    output.set_buffer(buffer); 
    
    return output;
}
// #include <iostream>
// #include <opencv4/opencv2/opencv.hpp>
// #include "media_handler.h"

// static auto MediaHandler::ConvertProtomatToMat(const ProtoMat& protomat) -> cv::Mat
// {
//     cv::Mat img = cv::Mat(cv::Size(protomat.width(), protomat.height()), 
//                             protomat.type());
//     string serializedMatrix(protomat.buffer());
//     int idx = 0;

//     /*
//     [질문]
//     if문을 바깥으로 꺼내두면 채널 확인을 한 번만 하면 된다
//     하지만 코드의 간결함을 위해 nested for loop 안으로 넣으면 
//     width * height 만큼 해줘야 한다. 어떤 것을 선택해야 하는가?
//     */
//     if (protomat.channels() == GRAY)
//     {
//         for (auto i=0 ; i<img.size().height ; i++)
//         {
//             for (auto j=0 ; j<img.size().width ; j++)
//             {  
//                 img.at<uchar>(i, j) = serializedMatrix[idx++];
//             }
//         }
//     }
//     /*
//     [질문]
//     else를 쓰면 코드가 간결해지지만 COLOR인지에 대한 직접적인 명시가 없기 때문에
//     직관적이지 않다
//     else if (f.channels() == COLOR)
//     */
//     else
//     {
//         for (auto i=0 ; i<img.size().height ; i++)
//         {
//             for (auto j=0 ; j<img.size().width ; j++)
//             {  
//                 img.at<cv::Vec3b>(i, j)[0] = serializedMatrix[idx++];
//                 img.at<cv::Vec3b>(i, j)[1] = serializedMatrix[idx++];
//                 img.at<cv::Vec3b>(i, j)[2] = serializedMatrix[idx++];
//             }
//         }
//     }

//     return img;
// }

// static auto MediaHandler::ConvertMatToProtomat(const cv::Mat& image) -> ProtoMat
// {
//     ProtoMat result;
//     string buffer("");
    
//     result.set_width(image.size().width);
//     result.set_height(image.size().height);
//     result.set_channels(image.channels());
//     result.set_type(image.type());
//     result.set_seq(result.seq() + 1);
    
//     if (image.channels() == GRAY)
//     {
//         for (auto i=0 ; i<image.size().height ; i++)
//         {
//             for (auto j=0 ; j<image.size().width ; j++)
//             {
//                 buffer += static_cast<uchar>(image.at<uchar>(i, j));
//             }
//         }
//     }
//     else if (image.channels() == COLOR)
//     {
//         for (auto i=0 ; i<image.size().height ; i++)
//         {
//             for (auto j=0 ; j<image.size().width ; j++)
//             {               
//                 const cv::Vec3b& pixel = image.at<cv::Vec3b>(i, j);

//                 buffer += static_cast<uchar>(pixel[0]); // B
//                 buffer += static_cast<uchar>(pixel[1]); // G
//                 buffer += static_cast<uchar>(pixel[2]); // R
//             }
//         }
//     }

//     result.set_buffer(buffer); 
    
//     return result;
// }
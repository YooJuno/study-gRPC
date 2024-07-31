#include <iostream>
#include <opencv4/opencv2/opencv.hpp>
#include "mat.pb.h"

using namespace std;

// practice::Mat cvMatToProto(const Mat& mat) 
// {
//     practice::Mat proto_mat;

//     proto_mat.set_rowsize(mat.rows);
//     proto_mat.set_colsize(mat.cols);
//     proto_mat.set_encoding(mat.type());
//     proto_mat.set_data(mat.data, mat.total() * mat.elemSize());

//     return proto_mat;
// }

// Mat protoToCvMat(const Mat& proto_mat) 
// {
//     Mat mat(proto_mat.rows(), proto_mat.cols(), proto_mat.type());
    
//     memcpy(mat.data, proto_mat.data().data(), proto_mat.data().size());

//     return mat;
// }

int main()
{
    string imagePath = "../dataset/dog.jpeg";

    cv::Mat img = cv::imread(imagePath, cv::IMREAD_COLOR);

    if (img.empty())
    {
        cerr << "Could not open or find the image!" << endl;
        return -1;
    }
    cout << "Width : " << img.size().width << endl;
    cout << "Height : " << img.size().height << endl;
    cout << "Channels : " << img.channels() << endl;
    cout << "Type : " << img.type() << endl;

    practice::Mat protoImg;
    protoImg.set_width(img.size().width);
    protoImg.set_height(img.size().height);
    protoImg.set_chennels(img.channels());
    protoImg.set_type(img.type());

    cout << "Width : " << protoImg.width() << endl;
    cout << "Height : " << protoImg.height() << endl;
    cout << "Channels : " << protoImg.channels() << endl;
    cout << "Type : " << protoImg.type() << endl;


    string windowName = "Display Image";

    cv::namedWindow(windowName, cv::WINDOW_NORMAL);

    cv::imshow(windowName, img);

    cv::waitKey(0);

    return 0;
}

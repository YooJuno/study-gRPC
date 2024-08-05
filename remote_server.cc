#include "remote_message.grpc.pb.h"

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <iostream>
#include <string>
#include <random>
#include <opencv4/opencv2/opencv.hpp>
#include <fstream>

// #include "media_handler.h"

#define COLOR 3
#define GRAY 1

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using remote::RemoteCommunication;
using remote::Empty;
using remote::ProtoMat;

using namespace std;

ABSL_FLAG(uint16_t, port, 50051, "Server port for the service");

/*

[Reference] https://github.com/improvess/yOLOv4-opencv-cpp-python

The contents of this class are references from above link

*/
class YOLOv4
{
public:
    YOLOv4() 
    {
        yoloVersion.assign("v4-tiny");
        yoloFolderPath.assign("../../yolov4/");
        colors = {cv::Scalar(255, 255, 0), cv::Scalar(0, 255, 0), cv::Scalar(0, 255, 255), cv::Scalar(255, 0, 0)};
        net = LoadNet(false); // if use cuda : true
        model = std::make_unique<cv::dnn::DetectionModel>(net);
        model->setInputParams(1./255, cv::Size(416, 416), cv::Scalar(), true);   
        classList = LoadClassList();
    }

    auto LoadClassList() -> vector<std::string>
    {
        std::vector<std::string> classList;
        std::ifstream ifs(yoloFolderPath + "classes.txt");
        std::string line;
        
        while (getline(ifs, line))
            classList.push_back(line);
        
        return classList;
    }

    auto LoadNet(bool is_cuda) -> cv::dnn::Net
    {
        auto result = cv::dnn::readNetFromDarknet(yoloFolderPath +  + "yolo" + yoloVersion + ".cfg", yoloFolderPath +  + "yolo" + yoloVersion + ".weights");

        if (is_cuda) 
        {
            std::cout << "Attempty to use CUDA\n";
            result.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
            result.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA_FP16);
        } 
        else 
        {
            std::cout << "Running on CPU\n";
            result.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
            result.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        }

        return result;
    }

    auto DetectObject(cv::Mat frame) -> cv::Mat
    {       
        std::vector<int> classIds;
        std::vector<float> confidences;
        std::vector<cv::Rect> boxes;
        model->detect(frame, classIds, confidences, boxes, .2, .4);

        int detections = classIds.size();

        for (auto i = 0; i < detections; ++i) 
        {
            auto box = boxes[i];
            auto classId = classIds[i];
            const auto color = colors[classId % colors.size()];

            cv::rectangle(frame, box, color, 1);
            cv::rectangle(frame, cv::Point(box.x, box.y - 20), cv::Point(box.x + box.width, box.y), color, cv::FILLED);

            cv::putText(frame, classList[classId].c_str(), cv::Point(box.x, box.y - 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
        }

        return frame;
    }

private:
    vector<cv::Scalar> colors;
    string yoloVersion;
    string yoloFolderPath;
    cv::dnn::Net net;
    unique_ptr<cv::dnn::DetectionModel> model;
    vector<std::string> classList;
};

class MediaHandler
{
public:
    static auto ConvertProtomatToMat(const ProtoMat& protomat) -> cv::Mat
    {
        cv::Mat img = cv::Mat(cv::Size(protomat.width(), protomat.height()), protomat.type());
        string serializedMatrix(protomat.buffer());
        int idx = 0;

        /*
        [질문]
        if문을 바깥으로 꺼내두면 채널 확인을 한 번만 하면 된다
        하지만 코드의 간결함을 위해 nested for loop 안으로 넣으면 
        width * height 만큼 해줘야 한다. 어떤 것을 선택해야 하는가?
        */
        if (protomat.channels() == GRAY)
        {
            for (auto i=0 ; i<img.size().height ; i++)
            {
                for (auto j=0 ; j<img.size().width ; j++)
                {  
                    img.at<uchar>(i, j) = serializedMatrix[idx++];
                }
            }
        }
        /*
        [질문]
        else를 쓰면 코드가 간결해지지만 COLOR인지에 대한 직접적인 명시가 없기 때문에
        직관적이지 않다
        else if (f.channels() == COLOR)
        */
        else
        {
            for (auto i=0 ; i<img.size().height ; i++)
            {
                for (auto j=0 ; j<img.size().width ; j++)
                {  
                    img.at<cv::Vec3b>(i, j)[0] = serializedMatrix[idx++];
                    img.at<cv::Vec3b>(i, j)[1] = serializedMatrix[idx++];
                    img.at<cv::Vec3b>(i, j)[2] = serializedMatrix[idx++];
                }
            }
        }

        return img;
    }
    
    static auto ConvertMatToProtomat(const cv::Mat& image) -> ProtoMat
    {
        ProtoMat result;
        string buffer("");
        
        result.set_width(image.size().width);
        result.set_height(image.size().height);
        result.set_channels(image.channels());
        result.set_type(image.type());
        result.set_seq(result.seq() + 1);
        
        if (image.channels() == GRAY)
        {
            for (auto i=0 ; i<image.size().height ; i++)
            {
                for (auto j=0 ; j<image.size().width ; j++)
                {
                    buffer += static_cast<uchar>(image.at<uchar>(i, j));
                }
            }
        }
        else if (image.channels() == COLOR)
        {
            for (auto i=0 ; i<image.size().height ; i++)
            {
                for (auto j=0 ; j<image.size().width ; j++)
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
};

class Uploader final : public RemoteCommunication::Service 
{

public:
    Uploader() : _yolo()
    {}

    Status RemoteProcessImageWithRect(ServerContext* context, const ProtoMat* request, ProtoMat* reply) override
    {
        cv::Mat frame = MediaHandler::ConvertProtomatToMat(*request);

        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> dis(0, 255);

        cv::circle(frame, cv::Point(frame.cols/2, frame.rows/2), 50, cv::Scalar(dis(gen), dis(gen), dis(gen)), 3); 

        *reply = MediaHandler::ConvertMatToProtomat(frame);

        return Status::OK;
    }

    Status RemoteProcessImageWithYOLO(ServerContext* context, const ProtoMat* request, ProtoMat* reply) override
    {
        cv::Mat frame = MediaHandler::ConvertProtomatToMat(*request);

        *reply = MediaHandler::ConvertMatToProtomat(_yolo.DetectObject(frame));

        return Status::OK;
    }

private:
    YOLOv4 _yolo;
};

//////////////////////////////////////////////////////////////////////
//                               REMARK                             //
//////////////////////////////////////////////////////////////////////
//   void Runserver(uint16_t port, char* pathOfDatasetDir) is       //
//   reference code from grpc/example/cpp.                          //
//   but *builder.SetMax...Size* codes are                          //
//   generated by juno                                              //
//////////////////////////////////////////////////////////////////////
void RunServer(uint16_t port) 
{
    string serverAddress = absl::StrFormat("0.0.0.0:%d", port);

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    Uploader service;

    ServerBuilder builder;

    builder.SetMaxSendMessageSize(1024 * 1024 * 1024 /* == 1GiB */);
    builder.SetMaxReceiveMessageSize(1024 * 1024 * 1024 /* == 1GiB */);

    builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());

    builder.RegisterService(&service);
    
    unique_ptr<Server> server(builder.BuildAndStart());

    cout << "\nServer listening on " << serverAddress << endl;

    server->Wait();
}

int main(int argc, char** argv) 
{
    absl::ParseCommandLine(argc, argv);
    
    RunServer(absl::GetFlag(FLAGS_port));
    
    return 0;
}

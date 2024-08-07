#include "remote_message.grpc.pb.h"

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include <grpcpp/grpcpp.h>
#include <opencv4/opencv2/opencv.hpp>

#include "media_handler.h"

#include <iostream>
#include <string>

#define CIRCLE 0
#define YOLO 1

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using remote::RemoteCommunication;
using remote::ProtoMat;

using namespace std;

ABSL_FLAG(string, target, "localhost:50051", "Server address");

class ClientNode : public MediaHandler
{
public:
    ClientNode(shared_ptr<Channel> channel)
        : _stub (RemoteCommunication::NewStub(channel)) {}

    auto RemoteProcessImage (cv::Mat image, int job) -> cv::Mat
    {
        ProtoMat request, reply;
        ClientContext context;
        Status status;

        request = ConvertMatToProtomat(image);

        if (job == CIRCLE)
            status = _stub->RemoteProcessImageWithCircle(&context, request, &reply);
        else if (job == YOLO)
            status = _stub->RemoteProcessImageWithYOLO(&context, request, &reply);

        if (!status.ok())
        {   
            cout << "gRPC connection is unstable\n";
            exit(1);
        }

        return ConvertProtomatToMat(reply);
    }

private:
    unique_ptr<RemoteCommunication::Stub> _stub;
};

void RunClient(string targetStr, string videoPath, int job)
{
    ProtoMat protoMat;

    grpc::ChannelArguments args;
    args.SetMaxReceiveMessageSize(1024 * 1024 * 1024 /* == 1GiB */);
    args.SetMaxSendMessageSize(1024 * 1024 * 1024 /* == 1GiB */);

    ClientNode service(grpc::CreateCustomChannel(targetStr, grpc::InsecureChannelCredentials(), args));

    cv::VideoCapture cap(videoPath);
    int fps = cap.get(cv::CAP_PROP_FPS);
    int sequenceNum = 0;

    cv::Mat frame, processedFrame;

    while (cap.read(frame))
    {
        processedFrame = service.RemoteProcessImage(frame, job);

        cv::imshow("processed video", processedFrame);

        if (sequenceNum % fps == 0) // 1 image per 1 sec
        {
            string imagePath = "../result/Image_" + to_string(sequenceNum) + ".jpeg";
            cv::imwrite(imagePath.c_str(), processedFrame);
        }

        sequenceNum++;

        if (cv::waitKey(1000/fps) == 27) 
            break;
    }
}

int main(int argc, char** argv)  
{
    absl::ParseCommandLine(argc, argv);

    if (argc != 3)
    {
        cout << argv[0] << "   <VIDEO_PATH>   <Circle:0, YOLO:1>\n";
        return 0;
    }

    string videoPath(argv[1]);

    RunClient(absl::GetFlag(FLAGS_target), videoPath, atoi(argv[2]));

    return 0;
}

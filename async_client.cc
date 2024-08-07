#include "remote_message.grpc.pb.h"

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include <grpcpp/grpcpp.h>
#include <opencv4/opencv2/opencv.hpp>

#include "media_handler.h"

#include <iostream>
#include <string>

#include <condition_variable>  // cv //
#include <mutex>               // mu //

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using grpc::ClientAsyncResponseReader;
using grpc::CompletionQueue;

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
        CompletionQueue cq;

        request = ConvertMatToProtomat(image);

        std::unique_ptr<ClientAsyncResponseReader<ProtoMat> > rpc(
            _stub->AsyncRemoteProcessImageWithYOLO(&context, request, &cq));

        // Request that, upon completion of the RPC, "reply" be updated with the
        // server's response; "status" with the indication of whether the operation
        // was successful. Tag the request with the integer 1.
        rpc->Finish(&reply, &status, (void*)1);
        void* got_tag;
        bool ok = false;

        // Block until the next result is available in the completion queue "cq".
        // The return value of Next should always be checked. This return value
        // tells us whether there is any kind of event or the cq_ is shutting down.
        CHECK(cq.Next(&got_tag, &ok));

        // Verify that the result from "cq" corresponds, by its tag, our previous
        // request.
        CHECK_EQ(got_tag, (void*)1);

        // ... and that the request was completed successfully. Note that "ok"
        // corresponds solely to the request for updates introduced by Finish().
        CHECK(ok);

        // Act upon the status of the actual RPC.
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
        
        if (sequenceNum++ % fps == 0) // 1 image per 1 sec
        {
            string imagePath = "../result/Image_" + to_string(sequenceNum-1) + ".jpeg";
            cv::imwrite(imagePath.c_str(), processedFrame);
        }

        cv::imshow("processed video", processedFrame);
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

    RunClient(absl::GetFlag(FLAGS_target), argv[1], atoi(argv[2]));

    return 0;
}

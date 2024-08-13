#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/check.h"

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#include <opencv4/opencv2/opencv.hpp>
#include "remote_message.grpc.pb.h"
#include "image_handler.h"
#include "video_handler.h"

#include <iostream>
#include <string>
#include <vector>
#include <thread>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientAsyncResponseReader;
using grpc::CompletionQueue;

using remote::RemoteCommunication;
using remote::ProtoMat;
using remote::YoloData;

using namespace std;
using namespace cv;

// [Argument option]
// It can be seen with "--help" option
ABSL_FLAG(string, target, "localhost:50051", "Server address");
ABSL_FLAG(string, input_video_path, "../dataset/input_long.mp4", "Input video path");
ABSL_FLAG(string, output_video_path, "../dataset/output.avi", "Output video path"); // codec 문제로 확장자를 .avi로 하였음
ABSL_FLAG(uint32_t, job, 1, "Job(0:Circle , 1:YOLO)");


class ClientNode : public ImageHandler
{
public:
    ClientNode(shared_ptr<Channel> channel)
        : _stub(RemoteCommunication::NewStub(channel))
    {}

    void RunAsyncVideoProcessing(const string& path)
    {        
        VideoCapture cap(path);
        _videoHandler = new VideoHandler(cap);
        Mat frame;
        Mat nextFrame;
        cap.read(frame);

        int seq = 0;
        bool eof = false;

        while (!eof)
        {
            _videoHandler->PushBack(frame.clone());
            eof = !cap.read(nextFrame);

            AsyncClientCall* call = new AsyncClientCall;
            ProtoMat request = ConvertMatToProtoMat(frame);
            
            request.set_seq(seq++);
            request.set_eof(eof);
            
            call->response_reader = _stub->PrepareAsyncProcessYOLO(&call->context, request, &_cq);
            call->response_reader->StartCall();
            call->response_reader->Finish(&call->response, &call->status, (void*)call);

            frame = nextFrame;
        }
    }

    void WaitToReceiveResponse()
    {
        void* got_tag;
        bool ok = false;

        while (_cq.Next(&got_tag, &ok)) 
        {
            AsyncClientCall* call = static_cast<AsyncClientCall*>(got_tag);

            if (call->status.ok())
            {
                _videoHandler->PushBack(call->response);
                PrintProgress(call->response.seq(), _videoHandler->GetTotalCount() - 1);
            }

            if (call->response.eof())
                break;

            delete call;
        }
    }

    auto GetVideoHandler() -> VideoHandler*
    {
        return _videoHandler;
    }

    void PrintProgress(int currentSize, int totalSize)
    {   
        cout << "Processing " ;
        cout << "[";
        auto progressBarLength = 50;
        for(auto i=0; i<progressBarLength ; i++)
        {
            if (i<(int)((currentSize/(float)totalSize)*progressBarLength))
                cout << "#";
            else
                cout << " ";
        }
        cout << "]\r";

        if (currentSize/totalSize == 1) cout << "\n";
    }

private:
    struct AsyncClientCall 
    {
        YoloData response;
        Status status;
        ClientContext context;
        unique_ptr<ClientAsyncResponseReader<YoloData>> response_reader;
    };
    unique_ptr<RemoteCommunication::Stub> _stub;
    CompletionQueue _cq;
    VideoHandler* _videoHandler;
};

int main(int argc, char** argv)
{
    absl::ParseCommandLine(argc, argv);
    const string target_str = absl::GetFlag(FLAGS_target);
    
    grpc::ChannelArguments args;
    args.SetMaxReceiveMessageSize(1024 * 1024 * 1024);
    args.SetMaxSendMessageSize(1024 * 1024 * 1024);

    ClientNode service(grpc::CreateCustomChannel(target_str, grpc::InsecureChannelCredentials(), args));
    thread t(&ClientNode::WaitToReceiveResponse, &service);
    
    service.RunAsyncVideoProcessing(absl::GetFlag(FLAGS_input_video_path));

    t.join();

    auto videoHandler = service.GetVideoHandler();
    videoHandler->MergeYoloDataToVideo();
    videoHandler->PlayVideo();
    videoHandler->SaveVideoTo(absl::GetFlag(FLAGS_output_video_path));

    return 0;
}
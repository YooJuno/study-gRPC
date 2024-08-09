#include <opencv4/opencv2/opencv.hpp>
#include "remote_message.grpc.pb.h"
#include "media_handler.h"

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/check.h"

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <string>
#include <thread>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using grpc::ClientAsyncResponseReader;
using grpc::CompletionQueue;

using remote::RemoteCommunication;
using remote::ProtoMat;

using namespace std;

// [Argument option]
// It can be seen with "--help" option
ABSL_FLAG(std::string, target, "localhost:50051", "Server address");
ABSL_FLAG(std::string, video_path, "../dataset/video.mp4", "Video path");
ABSL_FLAG(uint32_t, job, 1, "Job(0:Circle , 1:YOLO)");

class ClientNode : public MediaHandler
{
public:
    explicit ClientNode(std::shared_ptr<Channel> channel)
        : stub_(RemoteCommunication::NewStub(channel))
        {}

    void AsyncProcessImage(const string& path) // 이름 바꿔야됨
    {        
        cv::VideoCapture cap(path);
        cv::Mat frame;
        cv::Mat nextFrame;

        int seq=0;
        string text;

        cap.read(frame);

        while (!_eof)
        {
            cv::putText(frame, "No. " + to_string(seq++), cv::Point(50, 100), 1, 4, cv::Scalar(200, 200, 200), 3, 8);

            AsyncClientCall* call = new AsyncClientCall;
            ProtoMat request = ConvertMatToProtoMat(frame);

            _eof = !cap.read(nextFrame);

            request.set_eof(_eof);
            request.set_seq(seq);
            
            // 실제로 전송을 하진 않고 준비만 하는 단계
            call->response_reader = stub_->PrepareAsyncProcessImage(&call->context, request, &cq_);
            
            // StartCall initiates the RPC call
            // 실제로 request 메세지가 전송되는 부분임. 여기서는 실제로 요청을 보냄.
            call->response_reader->StartCall();

            // 서버에서 responder_.Finish(reply_, Status::OK, this); 가 호출되면 
            // call->reply, call->status에 서버로부터의 응답이 저장되며, 큐에 call 인스턴스가 추가된다.
            // 여기서는 실제로 응답을 받진 않음. 받을 준비만 하고 블락킹하지 않는다.
            // 서버로부터 응답을 받으면, call에 응답을 넣고 call을 큐에 넣어준다.
            call->response_reader->Finish(&call->reply, &call->status, (void*)call);
            
            frame = nextFrame;        
        }
    }

    void AsyncCompleteRpc() 
    {
        void* got_tag;
        bool ok = false;

        // 실제로 값이 call에 저장되고, 그 call이 큐에 추가될 때 까지 기다리다가 들어오면 Blocking 해제
        // 시간적으로 가장 먼저 완료된 작업을 가져온다. 
        while (cq_.Next(&got_tag, &ok)) 
        {
            // The tag in this example is the memory location of the call object
            AsyncClientCall* call = static_cast<AsyncClientCall*>(got_tag);

            if (call->reply.eof())
            {
                cout << "2eof!\n";
                delete call;
                break;
            }

            CHECK(ok);

            if (call->status.ok())
            {
                cv::Mat processedImage = ConvertProtoMatToMat( call->reply);
                _videoWriter << processedImage;
                cv::imshow("client2",processedImage);
                if (cv::waitKey(1000/25) == 27)
                    break;
            }

            delete call;

        }
        cout << "End of File!\n";
    }

private:
    struct AsyncClientCall 
    {
        ProtoMat reply;
        ClientContext context;
        Status status;
        std::unique_ptr<ClientAsyncResponseReader<ProtoMat>> response_reader;
    };
    std::unique_ptr<RemoteCommunication::Stub> stub_;
    CompletionQueue cq_;

    float _fps;
    int width, height;
    cv::VideoWriter _videoWriter;
    bool _eof = false;
};

int main(int argc, char** argv) 
{
    absl::ParseCommandLine(argc, argv);
    const std::string target_str = absl::GetFlag(FLAGS_target);
    
    grpc::ChannelArguments args;
    args.SetMaxReceiveMessageSize(1024 * 1024 * 1024);
    args.SetMaxSendMessageSize(1024 * 1024 * 1024);

    ClientNode service(grpc::CreateCustomChannel(target_str, grpc::InsecureChannelCredentials(), args));

    thread t(&ClientNode::AsyncCompleteRpc, &service);

    service.AsyncProcessImage(absl::GetFlag(FLAGS_video_path));

    cout << "Complete request video" << endl;
    t.join();

    return 0;
}

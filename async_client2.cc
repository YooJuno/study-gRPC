#include "remote_message.grpc.pb.h"
#include <opencv4/opencv2/opencv.hpp>
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

// Argument option 관리
ABSL_FLAG(std::string, target, "localhost:50051", "Server address");

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using grpc::ClientAsyncResponseReader;
using grpc::CompletionQueue;

using remote::RemoteCommunication;
using remote::ProtoMat;

using namespace std;

class ClientNode : public MediaHandler
{
public:
    explicit ClientNode(std::shared_ptr<Channel> channel)
        : stub_(RemoteCommunication::NewStub(channel)) {}

    // Assembles the client's payload and sends it to the server.
    void ProcessImageWithYOLO(cv::Mat image) 
    {
        cout << "ProcessImageWithYOLO\n";
        // Data we are sending to the server.
        ProtoMat request = ConvertMatToProtomat(image);

        // Call object to store rpc data 
        AsyncClientCall* call = new AsyncClientCall;

        // stub_->PrepareAsyncSayHello() creates an RPC object, returning
        // an instance to store in "call" but does not actually start the RPC
        // Because we are using the asynchronous API, we need to hold on to
        // the "call" instance in order to get updates on the ongoing RPC.
        call->response_reader =
            stub_->PrepareAsyncRemoteProcessImageWithYOLO(&call->context, request, &cq_);

        // StartCall initiates the RPC call
        call->response_reader->StartCall();

        // Request that, upon completion of the RPC, "reply" be updated with the
        // server's response; "status" with the indication of whether the operation
        // was successful. Tag the request with the memory address of the call
        // object.
        call->response_reader->Finish(&call->reply, &call->status, (void*)call);
    }

    // Loop while listening for completed responses.
    // Prints out the response from the server.
    void AsyncCompleteRpc() 
    {
        void* got_tag;
        bool ok = false;

        // Block until the next result is available in the completion queue "cq".
        while (cq_.Next(&got_tag, &ok)) 
        {
            // The tag in this example is the memory location of the call object
            AsyncClientCall* call = static_cast<AsyncClientCall*>(got_tag);

            CHECK(ok);

            if (call->status.ok())
            {
                cv::imshow("client2",ConvertProtomatToMat( call->reply));
                cv::waitKey(0);
            }

            // Once we're complete, deallocate the call object.
            delete call;
        }
    }

private:
    // struct for keeping state and data information
    struct AsyncClientCall 
    {
        ProtoMat reply;
        ClientContext context;
        Status status;
        std::unique_ptr<ClientAsyncResponseReader<ProtoMat>> response_reader;
    };
    std::unique_ptr<RemoteCommunication::Stub> stub_;
    CompletionQueue cq_;
};

int main(int argc, char** argv) 
{
    // --target=
    absl::ParseCommandLine(argc, argv);
    std::string target_str = absl::GetFlag(FLAGS_target);
    
    // We indicate that the channel isn't authenticated 
    // (use of InsecureChannelCredentials()).
    grpc::ChannelArguments args;
    args.SetMaxReceiveMessageSize(1024 * 1024 * 1024);
    args.SetMaxSendMessageSize(1024 * 1024 * 1024);

    ClientNode service(grpc::CreateCustomChannel(target_str, grpc::InsecureChannelCredentials(), args));

    // Spawn reader thread that loops indefinitely
    // 새로운 스레드를 생성하면서 ClientNode 클래스의 AsyncCompleteRpc 멤버 함수를 실행
    // &ClientNode::AsyncCompleteRpc는 ClientNode 클래스의 멤버 함수 포인터를 나타냄
    // &service는 AsyncCompleteRpc 메서드를 호출할 객체(즉, ClientNode 클래스의 인스턴스)를 가리키는 포인터입니다.
    std::thread thread_ = std::thread(&ClientNode::AsyncCompleteRpc, &service);

    cv::VideoCapture cap(argv[1]);
    int fps = cap.get(cv::CAP_PROP_FPS);
    int sequenceNum = 0;

    cv::Mat frame;
    while (cap.read(frame))
    {
        service.ProcessImageWithYOLO(frame);
    }

    std::cout << "Press control-c to quit" << std::endl << std::endl;
    thread_.join();  // blocks forever

    return 0;
}

#include "remote_message.grpc.pb.h"

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <opencv4/opencv2/opencv.hpp>

#include "media_handler.h"
#include "yolov4.h"

#include <iostream>
#include <string>

#include <chrono>
#include <thread>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using grpc::ServerAsyncResponseWriter;
using grpc::ServerCompletionQueue;

using remote::RemoteCommunication;
using remote::ProtoMat;

using namespace std;

// [Argument option]
//       (type    , name, default, help-text)
ABSL_FLAG(uint16_t, port, 50051  , "Server port for the service");

YOLOv4 yolo;

class ServerImpl final
{
public:
    ~ServerImpl()
    {
        server_->Shutdown();
        // Always shutdown the completion queue after the server.
        cq_->Shutdown();
    }

    // There is no shutdown handling in this code.
    void Run(uint16_t port) 
    {
        std::string server_address = absl::StrFormat("0.0.0.0:%d", port);

        ServerBuilder builder;
        // Listen on the given address without any authentication mechanism.
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

        // Register "service_" as the instance through which we'll communicate with
        // clients. In this case it corresponds to an *asynchronous* service.
        builder.RegisterService(&service_);

        // Get hold of the completion queue used for the asynchronous communication
        // with the gRPC runtime.
        cq_ = builder.AddCompletionQueue();

        // Finally assemble the server.
        server_ = builder.BuildAndStart();
        std::cout << "Server listening on " << server_address << std::endl;

        // Proceed to the server's main loop.
        HandleRpcs();
    }

private:
    void HandleRpcs() 
    {
        // Spawn a new CallData instance to serve new clients.
        new CallData(&service_, cq_.get(), 0); // status = CREATE
        void* tag;  // 작업을 식별하는 태그 (일반적으로 CallData 객체의 포인터)
        bool ok;    // 작업이 성공적으로 완료되었는지를 나타내는 플래그
        
        // Block waiting to read the next event from the completion queue.
        while (true) // 탈출 조건?
        {    
            CHECK(cq_->Next(&tag, &ok)); // Completion Queue에서 이벤트 대기 (block loop)
            CHECK(ok); // 작업이 성공적으로 완료되었는지 확인

            static_cast<CallData*>(tag)->Proceed(); // tag로 식별된 작업을 처리.
        }

        cout<<"after while()\n";
    }

    // Class encompasing the state and logic needed to serve a request.
    class CallData : public MediaHandler
    {
    public:
        // Take in the "service" instance (in this case representing an asynchronous
        // server) and the completion queue "cq" used for asynchronous communication
        // with the gRPC runtime.
        CallData(RemoteCommunication::AsyncService* service, ServerCompletionQueue* cq, int id)
        : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE), _id(id)
        {
            // Invoke the serving logic right away.
            Proceed();
        }

        void Proceed() 
        {
            if (status_ == CREATE) 
            {
                // Make this instance progress to the PROCESS state.
                status_ = PROCESS;

                // As part of the initial CREATE state, we *request* that the system
                // start processing SayHello requests. In this request, "this" acts are
                // the tag uniquely identifying the request (so that different CallData
                // instances can serve different requests concurrently), in this case
                // the memory address of this CallData instance.
                // 클라이언트 요청을 비동기적으로 처리할 것을 시스템에 요청함
                // 요청이 완료되면, 이 정보는 Completion Queue에 이벤트로 기록됨
                service_->RequestRemoteProcessImageWithYOLO(&ctx_, &request_, &responder_, cq_, cq_, this);

                cout << "[" << _id << "]create" << endl;
            }
            else if (status_ == PROCESS) 
            { 
                cout << "[" << _id << "]    process" << endl;
                // Spawn a new CallData instance to serve new clients while we process
                // the one for this CallData. The instance will deallocate itself as
                // part of its FINISH state.
                new CallData(service_, cq_, _id + 1);

                // Delay for Debugging
                // this_thread::sleep_for(chrono::milliseconds(500));

                // The actual processing.
                cv::Mat frame = ConvertProtomatToMat(request_);
                reply_ = ConvertMatToProtomat(yolo.DetectObject(frame));
                
                // And we are done! Let the gRPC runtime know we've finished, using the
                // memory address of this instance as the uniquely identifying tag for
                // the event.
                status_ = FINISH;
                responder_.Finish(reply_, Status::OK, this);
            } 
            else 
            {
                cout << "[" << _id << "]        finish" << endl;
                CHECK_EQ(status_, FINISH);
                // Once in the FINISH state, deallocate ourselves (CallData).
                delete this;
            }
        }

    private:
        // The means of communication with the gRPC runtime for an asynchronous
        // server.
        RemoteCommunication::AsyncService* service_;

        // The producer-consumer queue where for asynchronous server notifications.
        ServerCompletionQueue* cq_;

        // Context for the rpc, allowing to tweak aspects of it such as the use
        // of compression, authentication, as well as to send metadata back to the
        // client.
        ServerContext ctx_;

        // What we get from the client.
        // What we send back to the client.
        ProtoMat request_, reply_;

        // The means to get back to the client.
        ServerAsyncResponseWriter<ProtoMat> responder_;

        // Let's implement a tiny state machine with the following states.
        enum CallStatus { CREATE, PROCESS, FINISH };
        CallStatus status_;  // The current serving state.

        int _id;
    };

    std::unique_ptr<ServerCompletionQueue> cq_;
    RemoteCommunication::AsyncService service_;
    std::unique_ptr<Server> server_;
};

int main(int argc, char** argv) 
{
    /* --target=<PORT_NUMBER> */
    absl::ParseCommandLine(argc, argv);
    ServerImpl server;
    server.Run(absl::GetFlag(FLAGS_port));
    
    return 0;
}

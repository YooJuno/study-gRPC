#include <opencv4/opencv2/opencv.hpp>
#include "remote_message.grpc.pb.h"
#include "media_handler.h"
#include "yolov4.h"

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

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
        cq_->Shutdown();
    }

    void Run(uint16_t port) 
    {
        std::string server_address = absl::StrFormat("0.0.0.0:%d", port);
        ServerBuilder builder;
    
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service_);

        cq_ = builder.AddCompletionQueue();
        server_ = builder.BuildAndStart();

        std::cout << "Server listening on " << server_address << std::endl;

        HandleRpcs();
    }

private:
    void HandleRpcs() 
    {
        // Spawn a new CallData instance to serve [new clients].
        new CallData(&service_, cq_.get(), 0); // 최초 1회만. 나머지는 Proceed()에서
        void* tag;  // 작업을 식별하는 태그 (일반적으로 CallData 객체의 포인터)
        bool ok;
        
        // Block waiting to read the next event from the completion queue.
        while (true)
        {    
            ///////////////////////////////////////
            ////////////   이해 안됨   /////////////
            ///////////////////////////////////////
            // cq_->Next(&tag, &ok)
            CHECK(cq_->Next(&tag, &ok)); // ==> if(false) exit();
            CHECK(ok); // 작업이 성공적으로 완료되었는지 확인

            static_cast<CallData*>(tag)->Proceed(); // tag로 식별된 작업을 처리.
        }
    }

    class CallData : public MediaHandler
    {
    public:
        // Take in the "service" instance (in this case representing an asynchronous
        // server) and the completion queue "cq" used for asynchronous communication
        // with the gRPC runtime.
        CallData(RemoteCommunication::AsyncService* service, ServerCompletionQueue* cq, int id)
        : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE), _id(id)
        {
            cout << "------------------------------------- New CallData\n";
            Proceed();
        }


        ///////////////////////////////////////
        ////////////   이해 안됨   /////////////
        ///////////////////////////////////////
        void Proceed() 
        {
            if (status_ == CREATE) 
            {
                status_ = PROCESS;

                cout << "[" << _id << "]create" << endl;
                // As part of the initial CREATE state, we *request* that the system
                // start processing ProtoMat requests. In this request, "this" acts are
                // the tag uniquely identifying the request (so that different CallData
                // instances can serve different requests concurrently), in this case
                // the memory address of this CallData instance.
                // 클라이언트 요청을 비동기적으로 처리할 것을 시스템에 요청함
                // 요청이 완료되면, 이 정보는 Completion Queue에 이벤트로 기록됨
                service_->RequestRemoteProcessImageWithYOLO(&ctx_, &request_, &responder_, cq_, cq_, this); // responder_를 넣어주는게 포인트

            }
            else if (status_ == PROCESS) 
            { 
                cout << "[" << _id << "]    process" << endl;
                // Spawn a new CallData instance to serve new clients while we process
                // the one for this CallData.
                new CallData(service_, cq_, _id + 1);

                // Delay for Debugging
                // this_thread::sleep_for(chrono::milliseconds(500));

                // The actual processing.
                cv::Mat frame = ConvertProtomatToMat(request_);
                reply_ = ConvertMatToProtomat(yolo.DetectObject(frame));
                
                // And we are done! Let the gRPC runtime know we've finished
                status_ = FINISH;
                responder_.Finish(reply_, Status::OK, this);
            } 
            else 
            {
                cout << "[" << _id << "]        finish" << endl;
                CHECK_EQ(status_, FINISH); // 단순히 비교를 위한 임시매크로

                // Once in the FINISH state, deallocate ourselves (CallData).
                delete this;
            }
        }

    private:
        RemoteCommunication::AsyncService* service_;
        ServerCompletionQueue* cq_;
        ProtoMat request_, reply_;
        ServerAsyncResponseWriter<ProtoMat> responder_;
        ServerContext ctx_;

        enum CallStatus { CREATE, PROCESS, FINISH };
        CallStatus status_;

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

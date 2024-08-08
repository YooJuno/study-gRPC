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
// It can be seen with "--help" option
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

        HandleRpcs(server_, cq_);
    }

private:
    void HandleRpcs(RemoteCommunication::AsyncService* service, ServerCompletionQueue* cq) 
    {
        /* 1 */new CallData(&service, cq.get(), 0);

        void* tag;  // 작업을 식별하는 태그. 주로 this로 값이 정해지며 이는 CallData 인스턴스의 주소.
        bool ok;
        
        while (true)
        {    
            /* 4(PROCESS) - d(PROCESS) - 8(FINISH) - h(FINISH)*/CHECK(cq->Next(&tag, &ok));
            /*
            cq->Next(&tag, &ok)
            -   이벤트가 들어올 때 까지 blocking
            -   현재 프로그램에서 이벤트는
                    1) [요쳥 완료] service_->RequestProcessImage(&ctx_, &request_, &responder_, cq_, cq_, this);
                    2) [응답 완료] responder_.Finish(reply_, Status::OK, this);
            -   void* tag = 큐에서 기다리고 있던 CallData 인스턴스의 주소를 리턴.
            */

            CHECK(ok); 

            /* 5(PROCESS) - e(process) - 9(FINISH) - i(FINISH)*/static_cast<CallData*>(tag)->Proceed();
        }
    }

    class CallData : public MediaHandler
    {
    public:
        CallData(RemoteCommunication::AsyncService* service, ServerCompletionQueue* cq, int seq)
        : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE), _seq(seq)
        {
            /* 2(CREATE) - b(CREATE)*/Proceed();
        }

        void Proceed() 
        {
            if (status_ == CREATE) 
            {
                status_ = PROCESS;

                cout << "[" << _seq << "]create" << endl;
                /* 3 - */service_->RequestProcessImage(&ctx_, &request_, &responder_, cq_, cq_, this);
                // => 비동기 요청을 큐에 등록하여 loop에서 이 작업을 대기함
                // => 별도의 쓰레드에서 기다리는 것인지는 모르겠음.
            }
            else if (status_ == PROCESS) 
            {
                cout << "[" << _seq << "]    process" << endl;
                // initiate new connection
                /* 6 - a - ...*/new CallData(service_, cq_, _seq + 1); // 곧 들어올 요청을 위해

                cv::Mat frame = ConvertProtoMatToMat(request_);
                reply_ = ConvertMatToProtoMat(yolo.DetectObject(frame));
                
                status_ = FINISH;
                /* 7 - g*/ responder_.Finish(reply_, Status::OK, this); // 클라이언트에게 전송 알림.
                // ServerCompletionQueue에 이벤트가 추가됨.
                // CREATE 에서 요청 등록할 때 responder_를 함께 넣어줬기 때문에 비동기 작업이 완료되면
                // cq_에 이벤트가 추가됨.
                // 여기서 추가되는 this 포인터는 현재 인스턴스의 주소이며 이는 Queue에 추가된다.
            } 
            /* 10 - j */else 
            {
                // Queue에 등록 되는 것이 없기 때문에 종료.
                cout << "[" << _seq << "]        finish" << endl;
                CHECK_EQ(status_, FINISH);

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

        int _seq;
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

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
#include <mutex>
#include <chrono>


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
        cq_2->Shutdown();
    }

    void Run(uint16_t port) 
    {
        std::string server_address = absl::StrFormat("0.0.0.0:%d", port);
        ServerBuilder builder;
    
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service_);

        cq_ = builder.AddCompletionQueue();
        cq_2 = builder.AddCompletionQueue();
        server_ = builder.BuildAndStart();

        std::cout << "Server listening on " << server_address << std::endl;

        HandleRpcs();
    }

private:
    void HandleRpcs() 
    {
        new CallData(&service_, cq_.get(), 0);

        void* tag;  // 작업을 식별하는 태그. 주로 this로 값이 정해지며 이는 CallData 인스턴스의 주소.
        bool ok;
        
        while (true)
        {    
            CHECK(cq_->Next(&tag, &ok));
            /*
            cq_->Next(&tag, &ok)
            -   이벤트가 들어올 때 까지 blocking
            -   현재 프로그램에서 이벤트는
                    1) [요쳥 완료] service_->RequestProcessImage(&ctx_, &request_, &responder_, cq_, cq_, this);
                    2) [응답 완료] responder_.Finish(reply_, Status::OK, this);
            -   void* tag = 큐에서 기다리고 있던 CallData 인스턴스의 주소를 tag로 리턴.
            */

            CHECK(ok); 

            static_cast<CallData*>(tag)->Proceed();
        }
    }
/*
[504] PROCESS -> FINISH : 261ms
[505] PROCESS -> FINISH : 212ms
[506] PROCESS -> FINISH : 164ms
[507] PROCESS -> FINISH : 111ms
[509] CREATE -> PROCESS : 52ms
[508] PROCESS -> FINISH : 103ms
[510] CREATE -> PROCESS : 51ms
[509] PROCESS -> FINISH : 103ms
[511] CREATE -> PROCESS : 52ms
[512] CREATE -> PROCESS : 51ms
[513] CREATE -> PROCESS : 45ms
[514] CREATE -> PROCESS : 51ms

[PROCESS -> FINISH] 수행 시간이 [CREATE -> PROCESS] 수행 시간의 두 배 이상이다. 
이로 인해 queue에는 PROCESS 시간동안 CREATE가 동시에 3~4개 씩 생기게 되고, 짧은 
시간 차이로 동시에 여러 개가 생성된 쓰레드에서는 동시에 여러개의 처리된 이미지를 큐에 넣게 되고
큐에서 클라이언트에게 보낼 때 순서가 엉키게 되는 것이다. 
간혹 500번 대의 이미지를 처리하던 도중 700 혹은 300번 대의 이미지가 나오는 경우도 있는데 이러한
이유 떄문인가 싶다.

*/
    class CallData : public MediaHandler
    {
    public:
        CallData(RemoteCommunication::AsyncService* service, ServerCompletionQueue* cq, int seq)
        : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE), _seq(seq)
        {
            Proceed();
        }

        void Proceed() 
        {
            if (status_ == CREATE) 
            {
                status_ = PROCESS;

                // => 비동기 요청을 큐에 등록하여 loop에서 이 작업을 대기함
                service_->RequestProcessImage(&ctx_, &request_, &responder_, cq_, cq_, this);

                _start = std::chrono::high_resolution_clock::now();
            }
            else if (status_ == PROCESS) 
            { 
                _end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(_end - _start);
                std::cout << "[" << _seq << "] CREATE -> PROCESS : " << duration.count() << "ms\n";

                _start = std::chrono::high_resolution_clock::now();

                new CallData(service_, cq_, _eof ? 0 : _seq + 1); // 곧 들어올 요청을 위해

                cv::Mat frame = ConvertProtoMatToMat(request_);
                reply_ = ConvertMatToProtoMat(yolo.DetectObject(frame));
                
                status_ = FINISH;

                // ServerCompletionQueue에 이벤트가 추가됨.
                // CREATE 에서 요청 등록할 때 responder_를 함께 넣어줬기 때문에 비동기 작업이 완료되면
                // cq_에 이벤트가 추가됨.
                // 여기서 추가되는 this 포인터는 현재 인스턴스의 주소이며 이는 Queue에 추가된다.
                responder_.Finish(reply_, Status::OK, this); // 클라이언트에게 전송 알림.
            } 
            else 
            {
                _end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(_end - _start);
                std::cout << "[" << _seq << "] PROCESS -> FINISH : " << duration.count() << "ms\n";

                // Queue에 등록 되는 것이 없기 때문에 종료.
                CHECK_EQ(status_, FINISH);

                delete this;
            }
        }

    private:
        RemoteCommunication::AsyncService* service_;
        ServerCompletionQueue* cq_;
        ServerCompletionQueue* cq2;
        ProtoMat request_, reply_;
        ServerAsyncResponseWriter<ProtoMat> responder_;
        ServerContext ctx_;

        std::chrono::time_point<std::chrono::high_resolution_clock> _start;
        std::chrono::time_point<std::chrono::high_resolution_clock> _end;

        enum CallStatus { CREATE, PROCESS, FINISH };
        CallStatus status_;

        int _seq = 0;
        int _eof = false;
    };

    std::unique_ptr<ServerCompletionQueue> cq_;
    std::unique_ptr<ServerCompletionQueue> cq_2;
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

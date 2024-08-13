
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <opencv4/opencv2/opencv.hpp>
#include "remote_message.grpc.pb.h"
#include "media_handler.h"
#include "yolov4.h"

#include <iostream>
#include <string>
#include <thread>
#include <mutex>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using grpc::ServerAsyncResponseWriter;
using grpc::ServerCompletionQueue;

using remote::RemoteCommunication;
using remote::ProtoMat;
using remote::YoloData;

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
        _server->Shutdown();
        _cq->Shutdown();
    }

    void Run(uint16_t port) 
    {
        string _serveraddress = absl::StrFormat("0.0.0.0:%d", port);
        ServerBuilder builder;
    
        builder.AddListeningPort(_serveraddress, grpc::InsecureServerCredentials());
        builder.RegisterService(&_service);

        _cq = builder.AddCompletionQueue();
        _server = builder.BuildAndStart();

        cout << "Server listening on " << _serveraddress << endl;

        HandleRpcs();
    }

private:
    void HandleRpcs() 
    {
        new CallData(&_service, _cq.get(), 0);

        void* tag;
        bool ok;
        
        while (true)
        {    
            CHECK(_cq->Next(&tag, &ok));
            CHECK(ok); 

            static_cast<CallData*>(tag)->Proceed();
        }
    }

    class CallData : public MediaHandler
    {
    public:
        CallData(RemoteCommunication::AsyncService* service, ServerCompletionQueue* cq, int seq)
            : _service(service), _cq(cq), _responder(&_ctx), _status(CREATE), _seq(seq), _eof(false)
        {
            Proceed();
        }

        void Proceed() 
        {
            if (_status == CREATE) 
            {
                _service->RequestProcessYOLO(&_ctx, &_request, &_responder, _cq, _cq, this);
                _status = PROCESS;
            }
            else if (_status == PROCESS) 
            { 
                new CallData(_service, _cq, _eof ? 0 : _seq + 1);

                cv::Mat frame(ConvertProtoMatToMat(_request));
                YoloData response = yolo.DetectYOLO(frame);
                response.set_seq(_request.seq());
                response.set_eof(_request.eof());
                
                _responder.Finish(response, Status::OK, this);
                _status = FINISH;
            }
            else
            {
                CHECK_EQ(_status, FINISH);
                delete this;
            }
        }

    private:
        RemoteCommunication::AsyncService* _service;
        ServerCompletionQueue* _cq;
        ProtoMat _request;

        ServerAsyncResponseWriter<YoloData> _responder;
        ServerContext _ctx;

        enum CallStatus { CREATE, PROCESS, FINISH };
        CallStatus _status;

        int _seq = 0;
        int _eof;
    };

    unique_ptr<ServerCompletionQueue> _cq;
    RemoteCommunication::AsyncService _service;
    unique_ptr<Server> _server;
};

int main(int argc, char** argv) 
{
    /* --target=<PORT_NUMBER> */
    absl::ParseCommandLine(argc, argv);
    ServerImpl server;
    server.Run(absl::GetFlag(FLAGS_port));
    
    return 0;
}

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
#include <random>
#include <fstream>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using grpc::ServerAsyncResponseWriter;
using grpc::ServerCompletionQueue;

using remote::RemoteCommunication;
using remote::ProtoMat;

using namespace std;

ABSL_FLAG(uint16_t, port, 50051, "Server port for the service");

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
    // Class encompasing the state and logic needed to serve a request.
    class CallData : public MediaHandler
    {
    public:
        // Take in the "service" instance (in this case representing an asynchronous
        // server) and the completion queue "cq" used for asynchronous communication
        // with the gRPC runtime.
        CallData(RemoteCommunication::AsyncService* service, ServerCompletionQueue* cq)
        : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE)
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
                service_->RequestRemoteProcessImageWithYOLO(&ctx_, &request_, &responder_, cq_, cq_, this);
            }
            else if (status_ == PROCESS) 
            {   
                cout << "PROCESS\n";
                // Spawn a new CallData instance to serve new clients while we process
                // the one for this CallData. The instance will deallocate itself as
                // part of its FINISH state.
                new CallData(service_, cq_);

                // The actual processing.
                cv::Mat frame = ConvertProtomatToMat(request_);
                reply_ = request_;
                // reply_ = ConvertMatToProtomat(_yolo.DetectObject(frame));

                
                // And we are done! Let the gRPC runtime know we've finished, using the
                // memory address of this instance as the uniquely identifying tag for
                // the event.
                status_ = FINISH;
                responder_.Finish(reply_, Status::OK, this);
            } 
            else 
            {
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
    };

    // This can be run in multiple threads if needed.
    void HandleRpcs() 
    {
        // Spawn a new CallData instance to serve new clients.
        new CallData(&service_, cq_.get());
        void* tag;  // uniquely identifies a request.
        bool ok;
        while (true) 
        {
            // Block waiting to read the next event from the completion queue. The
            // event is uniquely identified by its tag, which in this case is the
            // memory address of a CallData instance.
            // The return value of Next should always be checked. This return value
            // tells us whether there is any kind of event or cq_ is shutting down.
            CHECK(cq_->Next(&tag, &ok));
            CHECK(ok);
            static_cast<CallData*>(tag)->Proceed();
        } // 다음 작업을 불러옴
    }

    std::unique_ptr<ServerCompletionQueue> cq_;
    RemoteCommunication::AsyncService service_;
    std::unique_ptr<Server> server_;
};


// class ServerNode final 
// // : public RemoteCommunication::Service, public MediaHandler, public YOLOv4
// // RemoteCommunication::CallbackService 이거는 어디서 만들어 진거지?
// : public RemoteCommunication::CallbackService, public MediaHandler, public YOLOv4
// {
// public:
//     ServerNode()
//         : _yolo(new YOLOv4()) {}
//     Status RemoteProcessImageWithCircle(ServerContext* context, const ProtoMat* request, ProtoMat* reply) override
//     {
//         cv::Mat frame = ConvertProtomatToMat(*request);

//         random_device rd;
//         mt19937 gen(rd());
//         uniform_int_distribution<int> dis(0, 255);

//         cv::circle(frame, cv::Point(frame.cols/2, frame.rows/2), 50, cv::Scalar(dis(gen), dis(gen), dis(gen)), 3); 
//         *reply = ConvertMatToProtomat(frame);

//         return Status::OK;
//     }
    
//     ServerUnaryReactor* RemoteProcessImageWithYOLO(CallbackServerContext* context, const ProtoMat* request, ProtoMat* reply) override
//     {
//         cv::Mat frame = ConvertProtomatToMat(*request);
//         *reply = ConvertMatToProtomat(_yolo->DetectObject(frame));

//         ServerUnaryReactor* reactor = context->DefaultReactor();
//         reactor->Finish(Status::OK);
//         return reactor;
//     }
    

// private:
//     YOLOv4* _yolo;
// };

//////////////////////////////////////////////////////////////////////
//                               REMARK                             //
//////////////////////////////////////////////////////////////////////
//                                                                  //
//   void Runserver(uint16_t port) is                               //
//   reference code from grpc/example/cpp.                          //
//   but *builder.SetMax...Size* codes are                          //
//   generated by juno                                              //
//                                                                  //
//////////////////////////////////////////////////////////////////////
// void RunServer(uint16_t port) 
// {
//     ServerNode service;
//     ServerBuilder builder;
//     string serverAddress = absl::StrFormat("0.0.0.0:%d", port);

//     grpc::EnableDefaultHealthCheckService(true);
//     grpc::reflection::InitProtoReflectionServerBuilderPlugin();

//     builder.SetMaxSendMessageSize(1024 * 1024 * 1024 /* == 1GiB */);
//     builder.SetMaxReceiveMessageSize(1024 * 1024 * 1024 /* == 1GiB */);
//     builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
//     builder.RegisterService(&service);
    
//     unique_ptr<Server> server(builder.BuildAndStart());

//     cout << "\nServer listening on " << serverAddress << endl;

//     server->Wait();
// }

int main(int argc, char** argv) 
{
    absl::ParseCommandLine(argc, argv);
    ServerImpl server;
    server.Run(absl::GetFlag(FLAGS_port));
    
    return 0;
}

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
#include <vector>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using grpc::ClientAsyncResponseReader;
using grpc::CompletionQueue;

using remote::RemoteCommunication;
using remote::ProtoMat;
using remote::DetectedList;

using namespace std;
using namespace cv;

// [Argument option]
// It can be seen with "--help" option
ABSL_FLAG(string, target, "localhost:50051", "Server address");
ABSL_FLAG(string, video_path, "../dataset/video.mp4", "Video path");
ABSL_FLAG(uint32_t, job, 1, "Job(0:Circle , 1:YOLO)");

class ClientNode : public MediaHandler
{
public:
    explicit ClientNode(shared_ptr<Channel> channel)
        : _stub(RemoteCommunication::NewStub(channel)) {}

    void AsyncProcessImage(const string& path) // 이름 바꿔야됨
    {        
        Mat frame;
        Mat nextFrame;

        int seq=0;
        string text;

        VideoCapture cap(path);
        cap.read(frame);

        while (cap.read(nextFrame))
        {
            // putText(frame, "No. " + to_string(seq++), Point(50, 100), 1, 4, Scalar(200, 200, 200), 3, 8);
            _images.push_back(frame);
            AsyncClientCall* call = new AsyncClientCall;
            ProtoMat request = ConvertMatToProtoMat(frame);
            cout << seq << endl;
            request.set_seq(seq++);
            
            call->response_reader = _stub->PrepareAsyncProcessYOLO(&call->context, request, &_cq);

            call->response_reader->StartCall();

            call->response_reader->Finish(&call->response, &call->status, (void*)call);

            frame = nextFrame;
        }
    }

    void AsyncCompleteRpc()
    {
        void* got_tag;
        bool ok = false;

        while (_cq.Next(&got_tag, &ok)) 
        {
            AsyncClientCall* call = static_cast<AsyncClientCall*>(got_tag);

            CHECK(ok);

            if (call->status.ok())
            {
                DetectedList result = call->response;

                Scalar color(200, 200, 200);
                for (auto i : result.boxes())
                {
                //     cout << result.seq() << " (";
                //     cout << i.tl_x() << ", " ;
                //     cout << i.tl_y() << ", " ;
                //     cout << i.width() << ", " ; 
                //     cout << i.height() << ", " ; 
                //     cout << i.classname() << ", " ; 
                //     cout << (int)(100 * i.confidence()) << ")\n" ; 
                    Rect rect(i.tl_x(), i.tl_y(), i.width(), i.height());
                    rectangle(_images[result.seq()], rect, color, 1);
                }

                imshow("test", _images[result.seq()]);
                waitKey(1000/25);
            }

            delete call;
        }
        cout << "End of File!\n";
    }

private:
    struct AsyncClientCall 
    {
        DetectedList response;
        ClientContext context;
        Status status;
        unique_ptr<ClientAsyncResponseReader<DetectedList>> response_reader;
    };
    unique_ptr<RemoteCommunication::Stub> _stub;
    CompletionQueue _cq;

    vector<cv::Mat> _images;
};

int main(int argc, char** argv)
{
    absl::ParseCommandLine(argc, argv);
    const string target_str = absl::GetFlag(FLAGS_target);
    
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
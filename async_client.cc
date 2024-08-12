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
using remote::DetectedBoxList;
using remote::BoundingBox;

using namespace std;
using namespace cv;

// [Argument option]
// It can be seen with "--help" option
ABSL_FLAG(string, target, "localhost:50051", "Server address");
ABSL_FLAG(string, video_path, "../dataset/video.mp4", "Video path");
ABSL_FLAG(uint32_t, job, 1, "Job(0:Circle , 1:YOLO)");

class VideoMaker
{
public:
    VideoMaker(VideoCapture cap)
        : _cap(cap)
    {   
        _width      = cap.get(CAP_PROP_FPS);
	    _height     = cap.get(CAP_PROP_FRAME_WIDTH);
	    _fps        = cap.get(CAP_PROP_FRAME_HEIGHT);
        _count      = cap.get(CAP_PROP_FRAME_COUNT);
    }

    void PushBack(Mat image)
    {
        _images.push_back(image);
    }

    void PushBack(vector<BoundingBox> boxes)
    {
        _boxes.push_back(boxes);
    }

    void EncodeAndSave(const string& outputVideoPath)
    {
        VideoWriter output(outputVideoPath, VideoWriter::fourcc('M', 'P', '4', 'V'), _fps , Size(_width, _height), true);
        
        if (!output.isOpened())
        {
            cout << "에러 처리 요망\n" << endl;
            return ;
        }
        
        for(auto& img : _images)
            output << img;
    }

    auto GetImageWithIndex(int idx) -> Mat
    {   
        return _images[idx];
    }

    void ReplaceImageWith(Mat substitute, int idx)
    {
        _images[idx] = substitute;
    }

    auto GetImages() -> vector<Mat>
    {
        return _images;
    }

    // auto GetCap() -> VideoCapture
    // {

    // }
private:
    vector<vector<BoundingBox>> _boxes;
    vector<Mat> _images;
    VideoCapture _cap;

    int _width;
    int _height;
    int _fps;
    int _count;
};

class ClientNode : public MediaHandler
{
public:
    ClientNode(shared_ptr<Channel> channel)
        : _stub(RemoteCommunication::NewStub(channel))
    {}

    void AsyncProcessVideo(const string& path) // 이름 바꿔야됨
    {        
        VideoCapture cap(path);
        _maker = new VideoMaker(cap);
        Mat frame;
        Mat nextFrame;
        cap.read(frame);

        int seq = 0;
        bool eof = false;

        while (!eof)
        {
            _maker->PushBack(frame);
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
                cout << "SEQ : " << call->response.seq();
                cout << "   EOF : " << call->response.eof() << endl;
                vector<BoundingBox> boxes(call->response.boxes().begin(), call->response.boxes().end());
                _maker->PushBack(boxes);
            }

            if (call->response.eof())
                break;

            delete call;
        }
        cout << "End of File!\n";
    }

    auto GetVideoMaker() -> VideoMaker*
    {
        return _maker;
    }

private:
    struct AsyncClientCall 
    {
        DetectedBoxList response;
        ClientContext context;
        Status status;
        unique_ptr<ClientAsyncResponseReader<DetectedBoxList>> response_reader;
    };
    unique_ptr<RemoteCommunication::Stub> _stub;
    CompletionQueue _cq;

    VideoMaker* _maker;
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
    
    service.AsyncProcessVideo(absl::GetFlag(FLAGS_video_path));

    t.join();

    cout << "Complete!\n";
    // auto images = service.GetVideoMaker()->GetImages();
    // cout << images.size() << endl;
    // for (auto& img : images)
    // {
    //     imshow("test", img);
    //     waitKey(1000/25);
    // } 

    return 0;
}
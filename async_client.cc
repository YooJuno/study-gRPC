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
using remote::YoloData;
using remote::BoundingBox;

using namespace std;
using namespace cv;

// [Argument option]
// It can be seen with "--help" option
ABSL_FLAG(string, target, "localhost:50051", "Server address");
ABSL_FLAG(string, input_video_path, "../dataset/input_long.mp4", "Input video path");
ABSL_FLAG(string, output_video_path, "../dataset/output.avi", "Output video path"); // codec 문제로 확장자를 .avi로 하였음
ABSL_FLAG(uint32_t, job, 1, "Job(0:Circle , 1:YOLO)");

class VideoMaker
{
public:
    VideoMaker(VideoCapture cap)
        : _cap(cap)
    {   
        _fps        = cap.get(CAP_PROP_FPS);
	    _width      = cap.get(CAP_PROP_FRAME_WIDTH);
	    _height     = cap.get(CAP_PROP_FRAME_HEIGHT);
        _count      = cap.get(CAP_PROP_FRAME_COUNT);
        _colors = {Scalar(255, 255, 0), Scalar(0, 255, 0), Scalar(0, 255, 255), Scalar(255, 0, 0), Scalar(100,255,100)};
    }

    void PlayVideo()
    {
        for(const auto& image : _images)
        {
            imshow("video", image);
            waitKey(1000/_fps);
        }
        destroyWindow("video");
    }

    void SaveVideoTo(const string& path)
    {
        VideoWriter videowriter(path, VideoWriter::fourcc('X', 'V', 'I', 'D'), _fps , Size(_width, _height), true);

        if (!videowriter.isOpened())
            return;

        for(const auto& image : _images)
            videowriter << image;
    }

    void MergeYoloDataToImage()
    {
        for(auto& yolo : _yoloDataList)
        {
            for (int i = 0; i < yolo.boxes_size(); i++)
            {
                Rect box(yolo.boxes(i).tl_x(), yolo.boxes(i).tl_y(), yolo.boxes(i).width(), yolo.boxes(i).height());
                Rect textBox(yolo.boxes(i).tl_x(), yolo.boxes(i).tl_y() - 20, yolo.boxes(i).width(), 20);
                const auto color = _colors[yolo.boxes(i).classid() % _colors.size()];
                rectangle(_images[yolo.seq()], box, color, 2);
                rectangle(_images[yolo.seq()], textBox, color, FILLED);
                putText(_images[yolo.seq()], yolo.boxes(i).classname(), Point(yolo.boxes(i).tl_x(), yolo.boxes(i).tl_y() - 5), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0));
            }
        }
    }

    void PushBack(const Mat& image)
    {
        _images.push_back(image.clone());
    }

    void PushBack(YoloData yolo)
    {
        _yoloDataList.push_back(yolo);
    }

private:
    vector<YoloData> _yoloDataList;
    vector<Scalar> _colors;
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

    void RunAsyncVideoProcessing(const string& path)
    {        
        VideoCapture cap(path);
        _videoMaker = new VideoMaker(cap);
        Mat frame;
        Mat nextFrame;
        cap.read(frame);

        int seq = 0;
        bool eof = false;

        while (!eof)
        {
            _videoMaker->PushBack(frame.clone());
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

    void ReceiveResponse()
    {
        void* got_tag;
        bool ok = false;

        while (_cq.Next(&got_tag, &ok)) 
        {
            AsyncClientCall* call = static_cast<AsyncClientCall*>(got_tag);


            if (call->status.ok())
                _videoMaker->PushBack(call->response);

            if (call->response.eof())
                break;

            delete call;
        }
        cout << "End of File!\n";
    }

    auto GetVideoMaker() -> VideoMaker*
    {
        return _videoMaker;
    }

private:
    struct AsyncClientCall 
    {
        YoloData response;
        Status status;
        ClientContext context;
        unique_ptr<ClientAsyncResponseReader<YoloData>> response_reader;
    };
    unique_ptr<RemoteCommunication::Stub> _stub;
    CompletionQueue _cq;
    VideoMaker* _videoMaker;
};

int main(int argc, char** argv)
{
    absl::ParseCommandLine(argc, argv);
    const string target_str = absl::GetFlag(FLAGS_target);
    
    grpc::ChannelArguments args;
    args.SetMaxReceiveMessageSize(1024 * 1024 * 1024);
    args.SetMaxSendMessageSize(1024 * 1024 * 1024);

    ClientNode service(grpc::CreateCustomChannel(target_str, grpc::InsecureChannelCredentials(), args));
    thread t(&ClientNode::ReceiveResponse, &service);
    
    service.RunAsyncVideoProcessing(absl::GetFlag(FLAGS_input_video_path));

    t.join();

    auto videoMaker = service.GetVideoMaker();
    videoMaker->MergeYoloDataToImage();
    videoMaker->PlayVideo();
    videoMaker->SaveVideoTo(absl::GetFlag(FLAGS_output_video_path));

    return 0;
}
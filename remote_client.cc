#include "remote_message.grpc.pb.h"

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <string>
#include <opencv4/opencv2/opencv.hpp>

// #include "media_handler.h"

#define COLOR 3
#define GRAY 1

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using remote::RemoteCommunication;

using remote::Empty;
using remote::ProtoMat;

using namespace std;

ABSL_FLAG(string, target, "localhost:50051", "Server address");

class MediaHandler
{
public:
    static auto ConvertProtomatToMat(const ProtoMat& protomat) -> cv::Mat
    {
        cv::Mat img = cv::Mat(cv::Size(protomat.width(), protomat.height()), 
                                protomat.type());
        string serializedMatrix(protomat.buffer());
        int idx = 0;

        /*
        [질문]
        if문을 바깥으로 꺼내두면 채널 확인을 한 번만 하면 된다
        하지만 코드의 간결함을 위해 nested for loop 안으로 넣으면 
        width * height 만큼 해줘야 한다. 어떤 것을 선택해야 하는가?
        */
        if (protomat.channels() == GRAY)
        {
            for (auto i=0 ; i<img.size().height ; i++)
            {
                for (auto j=0 ; j<img.size().width ; j++)
                {  
                    img.at<uchar>(i, j) = serializedMatrix[idx++];
                }
            }
        }
        /*
        [질문]
        else를 쓰면 코드가 간결해지지만 COLOR인지에 대한 직접적인 명시가 없기 때문에
        직관적이지 않다
        else if (f.channels() == COLOR)
        */
        else
        {
            for (auto i=0 ; i<img.size().height ; i++)
            {
                for (auto j=0 ; j<img.size().width ; j++)
                {  
                    img.at<cv::Vec3b>(i, j)[0] = serializedMatrix[idx++];
                    img.at<cv::Vec3b>(i, j)[1] = serializedMatrix[idx++];
                    img.at<cv::Vec3b>(i, j)[2] = serializedMatrix[idx++];
                }
            }
        }

        return img;
    }

    static auto ConvertMatToProtomat(const cv::Mat& image) -> ProtoMat
    {
        ProtoMat result;
        string buffer("");
        
        result.set_width(image.size().width);
        result.set_height(image.size().height);
        result.set_channels(image.channels());
        result.set_type(image.type());
        result.set_seq(result.seq() + 1);
        
        if (image.channels() == GRAY)
        {
            for (auto i=0 ; i<image.size().height ; i++)
            {
                for (auto j=0 ; j<image.size().width ; j++)
                {
                    buffer += static_cast<uchar>(image.at<uchar>(i, j));
                }
            }
        }
        else if (image.channels() == COLOR)
        {
            for (auto i=0 ; i<image.size().height ; i++)
            {
                for (auto j=0 ; j<image.size().width ; j++)
                {               
                    const cv::Vec3b& pixel = image.at<cv::Vec3b>(i, j);

                    buffer += static_cast<uchar>(pixel[0]); // B
                    buffer += static_cast<uchar>(pixel[1]); // G
                    buffer += static_cast<uchar>(pixel[2]); // R
                }
            }
        }

        result.set_buffer(buffer); 
        
        return result;
    }
};

class Downloader : public MediaHandler
{
public:
    Downloader(shared_ptr<Channel> channel)
        : _stub (RemoteCommunication::NewStub(channel)) {}

    auto RemoteProcessImage (cv::Mat image) -> cv::Mat
    {
        ProtoMat request, reply;
        ClientContext context;

        request = ConvertMatToProtomat(image);

        Status status = _stub->RemoteProcessImage(&context, request, &reply);

        if (!status.ok())
        {   
            cout << "process error\n";
            exit(1);
        }

        return ConvertProtomatToMat(reply);
    }

private:
    unique_ptr<RemoteCommunication::Stub> _stub;
};

void RunClient(string targetStr, string videoPath)
{
    MediaHandler mediaHandler;
    ProtoMat protoMat;

    grpc::ChannelArguments args;
    args.SetMaxReceiveMessageSize(1024 * 1024 * 1024 /* == 1GiB */);
    args.SetMaxSendMessageSize(1024 * 1024 * 1024 /* == 1GiB */);
    Downloader service(grpc::CreateCustomChannel(targetStr, grpc::InsecureChannelCredentials(), args));

    cv::VideoCapture cap(videoPath);
    int fps = cap.get(cv::CAP_PROP_FPS);

    cout << fps << endl;
    cv::Mat frame;
    cv::Mat processedFrame;
    int sequenceNum = 0;
    while(cap.read(frame))
    {
        processedFrame = service.RemoteProcessImage(frame);

        cv::imshow("processed video", processedFrame);

        if(sequenceNum%fps==0)
        {
            string imagePath = "../../processed/Image_" + to_string(sequenceNum) + ".jpeg";
            cv::imwrite(imagePath.c_str(), processedFrame);
            cout << "Sequence Num : " << sequenceNum << endl;
        }
        sequenceNum++;
        if (cv::waitKey(1000/fps) == 27) 
            break;
    }
}

int main(int argc, char** argv)  
{
    absl::ParseCommandLine(argc, argv);

    if(argc != 2)
    {
        cout << "./remote_client <VIDEO_PATH>\n";
        return 0;
    }

    string videoPath(argv[1]);

    RunClient(absl::GetFlag(FLAGS_target), videoPath);

    return 0;
}

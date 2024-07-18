#include "remote_message.grpc.pb.h"

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <fstream> 
#include <tuple>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using remote::RemoteDownload;

using remote::RemoteRequest;
using remote::RemoteReply;

using remote::LoginInfo;
using remote::LoginResult;

using remote::DirEntries;

using remote::Empty;

using namespace std;

ABSL_FLAG(string, target, "localhost:50051", "Server address");

class DownloaderClient 
{
public:
    DownloaderClient(shared_ptr<Channel> channel)
        : _stub (RemoteDownload::NewStub(channel)) {}

    bool TryLoginToServer(const string& id, const string& pw)
    {
        ClientContext context;
        LoginInfo request;
        LoginResult reply;

        request.set_id(id);
        request.set_pw(pw);

        Status status = _stub->Login(&context, request, &reply);

        if (status.ok())
            return reply.result();
        else        
            exit(-1);
    }
    
    auto Download(const string& fileName) -> RemoteReply
    {
        ClientContext context;
        RemoteRequest request;
        RemoteReply reply;
        
        request.set_name(fileName);

        Status status = _stub->DownloadFile(&context, request, &reply); 

        if (status.ok())
            return reply; 
        else        
            exit(-1);
            
    }

    auto DownloadDirEntries() -> vector<string> 
    {   
        vector<string> output;
        ClientContext context;
        Empty request;
        DirEntries reply;

        Status status = _stub->DownloadDirEntries(&context, request, &reply);

        if (status.ok())
            for(auto i=0; i<reply.entries_size(); i++)
                output.push_back(reply.entries(i));
        else
            exit(-1);
        
        return output;
    }

private:
    unique_ptr<RemoteDownload::Stub> _stub;
};

auto inputLoginInfoByUser() -> pair<string, string> 
{   
    string id;
    string pw;

    cout << "ID : ";
    cin >> id;
    cout << "PW : ";
    cin >> pw;

    return make_pair(id, pw);
}

auto chooseContentFrom(vector<string> list) -> string
{
    int num;

    cout << "\n**** [List] ****\n";
    for (auto i=0; i<list.size(); i++)
        cout << "[" << i+1 << "] " << list[i] << endl;

    cout << "Choose what you wanna download \n: " ;
    cin >> num;

    return list[num-1];
}

void printReply(RemoteReply reply)
{
    const google::protobuf::Descriptor* descriptor = reply.GetDescriptor();
    const google::protobuf::Reflection* reflection = reply.GetReflection();

    cout << "\n***** [File info] *****\n";
    // i=1? => Assume that the 0th number is a file or folder.
    for (auto i=1; i<descriptor->field_count(); i++) 
    {
        const google::protobuf::FieldDescriptor* field = descriptor->field(i);
        cout << field->name() << " : ";

        if (field->type() == google::protobuf::FieldDescriptor::TYPE_INT32) 
            cout << reflection->GetInt32(reply, field) << endl;
        else if (field->type() == google::protobuf::FieldDescriptor::TYPE_STRING) 
            cout << reflection->GetString(reply, field) << endl;
        else if (field->type() == google::protobuf::FieldDescriptor::TYPE_BOOL) 
            cout << std::boolalpha << reflection->GetBool(reply, field) << endl;
        else 
            cout << "Unknown" << endl;
    }
}

void saveReplyTo(const string PathOfDownloadDir, RemoteReply reply)
{
    auto file = reply.file();
    auto file_name = reply.name();
    
    ofstream ofs;
    ofs.open(PathOfDownloadDir + file_name, ios::out | ios::binary);
    ofs.write((char *)file.data(), file.length());           
    ofs.close();
}

auto inputPathForSave() -> string
{
    string output;

    cout << "Input path where you wanna put your file (ex: ../../download/) \n: ";
    cin >> output;

    return output;
}

class GrpcService
{
public:
    int RunClient(int argc, char** argv)
    {
        absl::ParseCommandLine(argc, argv);
        auto targetStr = absl::GetFlag(FLAGS_target);
        
        return doRun(targetStr);
    }

protected:
    virtual int doRun(string targetStr)
    {
        DownloaderClient Downloader(grpc::CreateChannel(targetStr, grpc::InsecureChannelCredentials()));
        string userId;
        string userPw;
        bool permission = false;

        for(int cnt=0; cnt<3 & !permission; cnt++)
        {
            tie(userId, userPw) = inputLoginInfoByUser();
            permission = Downloader.TryLoginToServer(userId, userPw);
            if (!permission) 
                cout << "Incorrect ID or PW. Please retry\n";
        }
        
        if (permission)
        {
            auto datasetEntries = Downloader.DownloadDirEntries();
            auto fileName = chooseContentFrom(datasetEntries);
            
            auto reply = Downloader.Download(fileName);
            printReply(reply);
            saveReplyTo(inputPathForSave(), reply);
        }
        else
        {
            cout << "Incorrect input more than 3 times.\n";
            
            return 0;
        }

        return 0;
    }
};

int main(int argc, char** argv)  
{
    GrpcService service;

    return service.RunClient(argc, argv);
}

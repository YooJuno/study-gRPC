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

using remote::RemoteCommunication;

using remote::RemoteRequest;
using remote::RemoteReply;

using remote::UserLoginInfo;
using remote::LoginResult;

using remote::FileNamesOfDataset;

using remote::Empty;

using namespace std;

ABSL_FLAG(string, target, "localhost:50051", "Server address");

class Downloader 
{
public:
    Downloader(shared_ptr<Channel> channel)
        : _stub (RemoteCommunication::NewStub(channel)) {}

    bool TryLoginToServer(const string& id, const string& pw)
    {
        ClientContext context;
        UserLoginInfo request;
        LoginResult reply;

        request.set_id(id);
        request.set_pw(pw);

        Status status = _stub->LoginToServer(&context, request, &reply);

        if (status.ok())
            return reply.result();
                
        return false;
    }
    
    auto ChooseFileNameToDownload() -> string 
    {   
        vector<string> fileNames;
        int num;
        
        do
        {
            fileNames = GetFileNamesOfDataset();
        } 
        while (fileNames[0] == "error");
        
        cout << "\n**** [List] ****\n";
        for (auto i = 0; i < fileNames.size(); i++)
            cout << "[" << i+1 << "] " << fileNames[i] << endl;
    
        cout << "Choose you wanna download\n: " ;
        cin >> num;

        return fileNames[num-1];
    }
    
    auto GetFileNamesOfDataset() -> vector<string> 
    {   
        vector<string> result;
        ClientContext context;
        Empty request;
        FileNamesOfDataset reply;

        Status status = _stub->GetFileNamesOfDataset(&context, request, &reply);

        if (status.ok())
            for(auto i=0; i<reply.filenames_size(); i++)
                result.push_back(reply.filenames(i));
        else
            result.push_back("error");
        
        return result;
    }

    bool Download(const string& fileName)
    {
        ClientContext context;
        RemoteRequest request;

        request.set_name(fileName);

        Status status = _stub->DownloadFile(&context, request, &_reply); 

        if (status.ok())
            return true;
        else        
            return false;
    }

    void printReply()
    {
        const google::protobuf::Descriptor* descriptor = _reply.GetDescriptor();
        const google::protobuf::Reflection* reflection = _reply.GetReflection();

        cout << "\n***** [File info] *****\n";
        for (auto i=1; i<descriptor->field_count(); i++) 
        {
            const google::protobuf::FieldDescriptor* field = descriptor->field(i);
            cout << field->name() << " : ";

            if (field->type() == google::protobuf::FieldDescriptor::TYPE_INT32) 
                cout << reflection->GetInt32(_reply, field) << endl;
            else if (field->type() == google::protobuf::FieldDescriptor::TYPE_STRING) 
                cout << reflection->GetString(_reply, field) << endl;
            else if (field->type() == google::protobuf::FieldDescriptor::TYPE_BOOL) 
                cout << std::boolalpha << reflection->GetBool(_reply, field) << endl;
            else 
                cout << "Unknown" << endl;
        }
    }

    void saveReplyTo(const string PathOfDownload)
    {
        string file = _reply.buffer();
        string file_name = _reply.name();
        
        ofstream ofs;
        ofs.open(PathOfDownload + file_name, ios::out | ios::binary);
        ofs.write((char *)file.data(), file.length());           
        ofs.close();
    }

private:
    unique_ptr<RemoteCommunication::Stub> _stub;
    RemoteReply _reply;
};

auto getLoginInfoByUser() -> pair<string, string> 
{   
    string id;
    string pw;

    cout << "ID : ";
    cin >> id;
    cout << "PW : ";
    cin >> pw;

    return make_pair(id, pw);
}

auto GetPathOfDownload() -> string
{
    string result;

    cout << "Enter path where you wanna put your file (ex: ../../download/) \n: ";
    cin >> result;

    return result;
}

void RunClient(int argc, char** argv)
{
    absl::ParseCommandLine(argc, argv);
    auto targetStr = absl::GetFlag(FLAGS_target);

    Downloader service(grpc::CreateChannel(targetStr, grpc::InsecureChannelCredentials()));
    string userId;
    string userPw;
    bool permission = false;

    for(int cnt=0; cnt<3 & !permission; cnt++)
    {
        tie(userId, userPw) = getLoginInfoByUser();
        permission = service.TryLoginToServer(userId, userPw);
        if (!permission) 
            cout << "Incorrect ID or PW. Please retry\n";
    }
    
    if (permission)
    {   
        bool isDownloaded;
        do
        {
            auto fileName = service.ChooseFileNameToDownload();
            isDownloaded = service.Download(fileName);
            if(!isDownloaded)
                cout << "Can't Download [" << fileName << "]. Please retry.\n";
        } 
        while (!isDownloaded);
        
        service.printReply();

        auto pathOfDownload = GetPathOfDownload();
        if (pathOfDownload[pathOfDownload.length()-1] != '/')
            pathOfDownload += '/';

        service.saveReplyTo(pathOfDownload);
    }
    else
    {
        cout << "Incorrect access more than 3 times.\n";\
        exit(1);
    }
}

int main(int argc, char** argv)  
{
    RunClient(argc, argv);

    return 0;
}

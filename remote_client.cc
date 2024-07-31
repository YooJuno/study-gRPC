#include "remote_message.grpc.pb.h"

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <fstream> 

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientReader;

using remote::RemoteCommunication;
using remote::RemoteRequest;
using remote::File;
using remote::Header;
using remote::Data;
using remote::UserLoginInfo;
using remote::LoginResult;
using remote::FileNamesOfDataset;
using remote::Empty;

using remote::Data;

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

    auto GetFileNamesOfDataset() -> vector<string> 
    {   
        vector<string> result;
        ClientContext context;
        Empty request;
        FileNamesOfDataset reply;

        Status status = _stub->GetFileNamesOfDataset(&context, request, &reply);

        if (status.ok())
            for(const auto& i : reply.filenames())
                result.push_back(i);
        else
            result.push_back("error");
        
        return result;
    }
    
    auto SelectFileNameToDownload() -> string 
    {   
        vector<string> fileNames;
        int inputNum;
        int i;
        
        do
        {
            fileNames = GetFileNamesOfDataset();
        } 
        while (fileNames[0] == "error");
        
        cout << "\n**** [List] ****\n";
        for (i = 0; i < fileNames.size(); i++)
            cout << "[" << i+1 << "] " << fileNames[i] << endl;
        fileNames.push_back("quit");
        cout << "[" << i+1 << "] nothing to download(quit)" << endl;
        cout << "Select you wanna download\n: " ;
        cin >> inputNum;

        return fileNames[inputNum-1];
    }

    auto DownloadFile(const string& fileName, int chunkSize) -> File
    {
        ClientContext contextForHeader;
        ClientContext contextForData;
        RemoteRequest request;
        File reply;
        Header header;
        Data data;
        string queue("");

        request.set_name(fileName);
        request.set_chunksize(chunkSize);

        Status status = _stub->DownloadHeader(&contextForHeader, request, &header); 

        if (!status.ok())
        {
            reply.set_success(false);
            return reply;
        }

        std::unique_ptr<ClientReader<Data> > reader(_stub->DownloadData(&contextForData, request));

        while (reader->Read(&data)) 
        {
            queue += data.buffer();
            PrintProgress(queue.length(), header.size());
        }
        if(queue.length() != header.size())
        {
            reply.set_success(false);
            return reply;
        }

        status = reader->Finish();

        if (!status.ok())
        {
            reply.set_success(false);
            return reply;
        }

        reply.set_success(true);
        return reply;
    }

    void PrintProgress(int downloadedSize, int fullSize)
    {   
        cout << "Downloading " ;
        cout << "[";
        int progressBarLength = 40;
        for(int i=0; i<progressBarLength ; i++)
        {
            if (i<(int)((downloadedSize/(float)fullSize)*progressBarLength))
                cout << "#";
            else
                cout << " ";
        }
        cout << "]\r";

        if (downloadedSize/fullSize == 1) cout << "\n";
    }

    void SaveReplyTo(const string& PathOfDownload, const File file)
    {
        string buffer = file.data().buffer();
        string file_name = file.header().name();
        
        ofstream ofs;
        ofs.open(PathOfDownload + file_name, ios::out | ios::binary);
        ofs.write(buffer.c_str(), buffer.length());           
        ofs.close();
    }

private:
    unique_ptr<RemoteCommunication::Stub> _stub;
};

class IO
{
public:
    static auto GetLoginInfoByUser() -> pair<string, string> 
    {   
        cout << "**** [Login] ****\n";
        string id;
        string pw;

        cout << "ID : ";
        cin >> id;
        cout << "PW : ";
        cin >> pw;

        return make_pair(id, pw);
    }

    static auto GetPathOfDownload() -> string
    {
        string result;

        cout << "Enter path where you wanna save your file (ex: ../../download/) \n: ";
        cin >> result;

        return result;
    }

    static void PrintHeader(const Header& file)
    {
        const google::protobuf::Descriptor* descriptor = file.GetDescriptor();
        const google::protobuf::Reflection* reflection = file.GetReflection();

        cout << "\n***** [File info] *****\n";
        for (auto i=1; i<descriptor->field_count(); i++) 
        {
            const google::protobuf::FieldDescriptor* field = descriptor->field(i);
            cout << field->name() << " : ";

            if (field->type() == google::protobuf::FieldDescriptor::TYPE_INT32) 
                cout << reflection->GetInt32(file, field) << endl;
            else if (field->type() == google::protobuf::FieldDescriptor::TYPE_STRING) 
                cout << reflection->GetString(file, field) << endl;
            else if (field->type() == google::protobuf::FieldDescriptor::TYPE_BOOL) 
                cout << std::boolalpha << reflection->GetBool(file, field) << endl;
            else 
                cout << "Unknown" << endl;
        }
    }
};

void RunClient(string targetStr)
{
    grpc::ChannelArguments args;
    args.SetMaxSendMessageSize(4 * 1024 * 1024 /* == 4MB */);
    args.SetLoadBalancingPolicyName("round_robin");

    Downloader service(grpc::CreateCustomChannel(targetStr, grpc::InsecureChannelCredentials(), args));
    string userId;
    string userPw;
    bool permission = false;

    for (int cnt=0; cnt<3 && !permission; cnt++)
    {
        tie(userId, userPw) = IO::GetLoginInfoByUser();
        permission = service.TryLoginToServer(userId, userPw);
        if (!permission) 
            cout << "Login failed. Please retry\n\n";
    }

    if (permission)
    {
        File file;
        bool isDownloaded = false;

        for(auto cnt=0; cnt<3 && !isDownloaded; cnt++)
        {
            auto fileName = service.SelectFileNameToDownload();
            if (fileName == "quit")
            {
                cout << "Good bye\n";
                return ;
            }

            file = service.DownloadFile(fileName, 1024 /* 1 KiB/chunk */);

            isDownloaded = file.success();
            if(!isDownloaded)
                cout << "Can't download [" << fileName << "]. Please retry.\n\n";
        }
        
        IO::PrintHeader(file.header());

        auto pathOfDownload = IO::GetPathOfDownload();
        if (pathOfDownload[pathOfDownload.length()-1] != '/')
            pathOfDownload += '/';

        service.SaveReplyTo(pathOfDownload, file);
    }
    else
    {
        cout << "Incorrect access 3 times.\n";\
        exit(1);
    }
}

int main(int argc, char** argv)  
{
    absl::ParseCommandLine(argc, argv);
    RunClient(absl::GetFlag(FLAGS_target));

    return 0;
}

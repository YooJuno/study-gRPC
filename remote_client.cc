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

using remote::RemoteCommunication;
using remote::RemoteRequest;
using remote::File;
using remote::Header;
using remote::Data;
using remote::FileNamesOfDataset;
using remote::Empty;

using namespace std;

ABSL_FLAG(string, target, "localhost:50051", "Server address");

class InputAndOutput
{
public:
    static void PrintHeader(const Header& file)
    {
        const google::protobuf::Descriptor* descriptor = file.GetDescriptor();
        const google::protobuf::Reflection* reflection = file.GetReflection();

        cout << "\n***** [File info] *****\n";
        for (auto i=0; i<descriptor->field_count(); i++) 
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

    static void PrintFileNames(vector<string> fileNames)
    {
        int idx;
    
        cout << "\n**** [List] ****\n";
        for (idx = 0; idx < fileNames.size(); idx++)
            cout << "[" << idx+1 << "] " << fileNames[idx] << endl;
        cout << "[" << idx+1 << "] nothing to download(quit)" << endl;
    }

    static auto SelectFileNameFrom(vector<string> fileNames) -> string 
    {   
        int inputNum;
        cout << "Select you wanna download\n: " ;
        cin >> inputNum;

        return fileNames[inputNum-1];
    }
};

class Downloader 
{
public:
    Downloader(shared_ptr<Channel> channel)
        : _stub (RemoteCommunication::NewStub(channel)) {}

    auto GetFileNamesInDataset() -> vector<string> 
    {   
        vector<string> result;
        ClientContext context;
        Empty request;
        FileNamesOfDataset reply;

        Status status = _stub->GetFileNamesInDataset(&context, request, &reply);

        if (status.ok())
            for(const auto& i : reply.filenames())
                result.push_back(i);
        else
            result.push_back("error");
        
        return result;
    }

    auto DownloadFile(const string& fileName) -> File
    {
        ClientContext context;
        RemoteRequest request;
        File reply;
        string queue("");

        request.set_name(fileName);

        Status status = _stub->DownloadFile(&context, request, &reply); 

        if (!status.ok())
        {
            reply.set_success(false);
            return reply;
        }

        if(reply.header().name() != "" && reply.header().size() != 0 && reply.header().date() != "")
            reply.mutable_header()->set_success(true);

        if(reply.data().buffer() != "")
            reply.mutable_data()->set_success(true);

        reply.set_success(reply.header().success() && reply.data().success());

        return reply;
    }

    void SaveReplyTo(const string& pathOfDownloadDir, const File f)
    {
        string buffer = f.data().buffer();
        string f_name = f.header().name();
        
        ofstream ofs;

        if (pathOfDownloadDir[pathOfDownloadDir.length()-1] != '/')
            ofs.open(pathOfDownloadDir + '/' + f_name, ios::out | ios::binary);
        else
            ofs.open(pathOfDownloadDir + f_name, ios::out | ios::binary);

        ofs.write(buffer.c_str(), buffer.length());           
        ofs.close();
    }

private:
    unique_ptr<RemoteCommunication::Stub> _stub;
};

void RunClient(string targetStr, string pathOfDownloadDir)
{
    grpc::ChannelArguments args;
    args.SetMaxReceiveMessageSize(1024 * 1024 * 1024 /* == 1GiB */);
    args.SetMaxSendMessageSize(1024 * 1024 * 1024 /* == 1GiB */);

    Downloader service(grpc::CreateCustomChannel(targetStr, grpc::InsecureChannelCredentials(), args));

    File file;
    bool isDownloaded = false;

    for (auto cnt=0; cnt<3 && !isDownloaded; cnt++)
    {
        auto fileNames = service.GetFileNamesInDataset();
        
        if (fileNames[0] == "error")
        {
            cout << "The server connection terminated abnormally.\n";

            return ;
        }

        IO::PrintFileNames(fileNames);

        auto fileName = IO::SelectFileNameFrom(fileNames);

        file = service.DownloadFile(fileName);

        isDownloaded = file.success();
        if (!isDownloaded)
            cout << "Can't download [" << fileName << "]. Please retry.\n\n";
    }
    
    IO::PrintHeader(file.header());

    service.SaveReplyTo(pathOfDownloadDir, file);
}

int main(int argc, char** argv)  
{
    absl::ParseCommandLine(argc, argv);

    string pathOfDownloadDir(argv[1]);
    RunClient(absl::GetFlag(FLAGS_target), pathOfDownloadDir);

    return 0;
}

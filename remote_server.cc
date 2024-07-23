#include "remote_message.grpc.pb.h"

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <fstream> 
#include <ctime>
#include <sys/stat.h>
#include <dirent.h>

#define FILE (1)
#define Dir (2)

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerWriter;

using remote::RemoteCommunication;
using remote::RemoteRequest;
using remote::File;
using remote::UserLoginInfo;
using remote::LoginResult;
using remote::FileNamesOfDataset;
using remote::Empty;

using namespace std;

ABSL_FLAG(uint16_t, port, 50051, "Server port for the service");

auto GetPathOfDataset() -> string
{
    string result;

    cout << "Enter dataset path (ex: ../../dataset/)\n: ";
    cin >> result;

    return result;
}

auto GetFileNamesFrom(DIR* dir) -> vector<string>
{
    vector<string> result;
    struct dirent* entry;

    while ((entry = readdir(dir)))
    {
        string name = entry->d_name;
        if (name == "." || name == "..") continue;
        result.push_back(name);
    }
    rewinddir(dir);

    return result;
}

bool isDirOpened(DIR* dir)
{
    return dir;
}

class Uploader final : public RemoteCommunication::Service 
{
public:
    Uploader()
    {
        do
        {
            _pathOfDataset = GetPathOfDataset();
            if(_pathOfDataset[_pathOfDataset.length()-1] != '/')
                _pathOfDataset += '/';
            _dir = opendir(_pathOfDataset.c_str());

            if (!_dir)
                cout << "Can't open folder. Please retry.\n";
        }
        while (!isDirOpened(_dir));
    }

    Status LoginToServer(ServerContext* context, const UserLoginInfo* request, LoginResult* reply) 
    override
    {
        _userId = request->id();
        _userPw = request->pw();

        if (_userId == "juno" && _userPw == "980220") // 추후에 DB연동 시스템으로 확장.
            reply->set_result(true);
        else
            reply->set_result(false);

        return Status::OK;
    }

    Status GetFileNamesOfDataset(ServerContext* context, const Empty* request, FileNamesOfDataset* reply) 
    override 
    {
        auto fileNames = GetFileNamesFrom(_dir);
        
        for (const auto& i : fileNames)
            reply->add_filenames(i);

        return Status::OK;
    }

    Status DownloadFile(ServerContext* context, const RemoteRequest* request, File* reply)
    override 
    {
        ifstream ifs;
        string nameOfTargetFile(request->name());
        string pathOfTargetFile = _pathOfDataset + request->name();
        ifs.open(pathOfTargetFile, ios::binary);

        if(!ifs)
            cout << "failed to open file\n";

        string buffer((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());
        ifs.close();

        cout << buffer.size() << endl;

        reply->set_buffer(buffer);
        cout << reply->buffer().length()<<endl;
        reply->set_name(nameOfTargetFile);
        reply->set_size(buffer.length());
        
        struct stat attr;
        if (stat(pathOfTargetFile.c_str(), &attr) == 0) 
        {
            string creationTimeOfTargetFile = ctime(&attr.st_ctime);
            creationTimeOfTargetFile[creationTimeOfTargetFile.length()-1] = '\0';
            reply->set_date(creationTimeOfTargetFile); 
        }
        return Status::OK;
    }
    
    Status DownloadFileViaStream(ServerContext* context, const RemoteRequest* request, ServerWriter<File>* reply)
    override 
    {
        ifstream ifs;
        File chunk;
        struct stat attr;

        string nameOfTargetFile(request->name());
        string pathOfTargetFile = _pathOfDataset + request->name();
        ifs.open(pathOfTargetFile, ios::binary);

        if(!ifs)
            cout << "failed to open file\n";

        string buffer((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());
        ifs.close();

        chunk.set_buffer(buffer);
        chunk.set_name(nameOfTargetFile);
        chunk.set_size(buffer.length());
        
        if (stat(pathOfTargetFile.c_str(), &attr) == 0) 
        {
            string creationTimeOfTargetFile = ctime(&attr.st_ctime);
            creationTimeOfTargetFile[creationTimeOfTargetFile.length()-1] = '\0';
            chunk.set_date(creationTimeOfTargetFile); 
        }

        reply->Write(chunk);

        return Status::OK;
    }

private:
    string _userId;
    string _userPw;
    string _pathOfDataset;
    DIR* _dir;
};

void RunServer(uint16_t port) 
{
    std::string serverAddress = absl::StrFormat("0.0.0.0:%d", port);
    Uploader service;

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;

    builder.SetMaxSendMessageSize(1024 * 1024 * 1024);
    builder.SetMaxMessageSize(1024 * 1024 * 1024);
    builder.SetMaxReceiveMessageSize(1024 * 1024 * 1024);

    builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());

    builder.RegisterService(&service);
    
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << serverAddress << std::endl;

    server->Wait();
}

int main(int argc, char** argv) 
{
    absl::ParseCommandLine(argc, argv);
    RunServer(absl::GetFlag(FLAGS_port));
    
    return 0;
}

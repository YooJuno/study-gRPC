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

using remote::RemoteCommunication;

using remote::RemoteRequest;
using remote::RemoteReply;

using remote::UserLoginInfo;
using remote::LoginResult;

using remote::EntriesOfDataset;

using remote::Empty;

using namespace std;

ABSL_FLAG(uint16_t, port, 50051, "Server port for the service");

auto inputDirPath() -> string
{
    string output;

    cout << "input dataset path (ex: ../../dataset/)\n: ";
    cin >> output;

    return output;
}

auto getEntriesNameFrom(DIR* dir) -> vector<string>
{
    vector<string> output;
    struct dirent* entry;

    while ((entry = readdir(dir)))
    {
        string name = entry->d_name;
        if (name == "." || name == "..") continue;
        output.push_back(name);
    }
    rewinddir(dir);

    return output;
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
            _pathOfDataset = inputDirPath();
            if(_pathOfDataset[_pathOfDataset.length()-1] != '/')
                _pathOfDataset += '/';
            _dir = opendir(_pathOfDataset.c_str());
        }
        while (!isDirOpened(_dir));
    }

    Status LoginToServer(ServerContext* context, const UserLoginInfo* request, LoginResult* reply) 
    override
    {
        _userId = request->id();
        _userPw = request->pw();

        if (_userId == "juno" && _userPw == "980220") // 추후에 DB연동 시스템으로 확장 계획
            reply->set_result(true);
        else
            reply->set_result(false);

        return Status::OK;
    }

    Status DownloadEntriesNameOfDataset(ServerContext* context, const Empty* request, EntriesOfDataset* reply) 
    override 
    {
        auto entries = getEntriesNameFrom(_dir);
        
        for (auto i=0; i<entries.size(); i++)
            reply->add_entries(entries[i]);

        return Status::OK;
    }

    Status DownloadTargetFile(ServerContext* context, const RemoteRequest* request, RemoteReply* reply) // 파일?
    override 
    {
        ifstream ifs;
        string nameOfTargetFile(request->name());
        string pathOfTargetFile = _pathOfDataset + request->name();
        ifs.open(pathOfTargetFile, ios::binary);
        string buffer((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());

        reply->set_buffer(buffer);
        reply->set_name(nameOfTargetFile);
        reply->set_size(buffer.length());
        
        string creationTimeOfTargetFile;
        struct stat attr;
        if (stat(pathOfTargetFile.c_str(), &attr) == 0) 
            creationTimeOfTargetFile = ctime(&attr.st_ctime);

        reply->set_date(creationTimeOfTargetFile); 
        
        return Status::OK;
    }

private:
    string _userId;
    string _userPw;
    string _pathOfDataset;
    DIR* _dir;
};

void RunServer(uint16_t port) {
    std::string serverAddress = absl::StrFormat("0.0.0.0:%d", port);
    Uploader service;

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;

    builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());

    builder.RegisterService(&service);
    
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << serverAddress << std::endl;

    server->Wait();
}

int main(int argc, char** argv) {
    absl::ParseCommandLine(argc, argv);
    RunServer(absl::GetFlag(FLAGS_port));
    
    return 0;
}

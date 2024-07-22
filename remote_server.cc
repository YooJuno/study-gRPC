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

using remote::RemoteDownload;

using remote::RemoteRequest;
using remote::RemoteReply;

using remote::LoginInfo;
using remote::LoginResult;

using remote::DirEntries;

using remote::Empty;

using namespace std;

ABSL_FLAG(uint16_t, port, 50051, "Server port for the service");

auto inputDirPath() -> string
{
    string output;

    cout << "input dataset path (ex: ../../dataset/) \n: ";
    cin >> output;

    return output;
}

auto getEntriesFrom(DIR* dir) -> vector<string>
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

class DownloadServer final : public RemoteDownload::Service 
{
public:
    DownloadServer()
    {
        do
        {
            _pathOfDatasetDir = inputDirPath();
            if(_pathOfDatasetDir[_pathOfDatasetDir.length()-1] != '/')
                _pathOfDatasetDir += '/';
            _dir = opendir(_pathOfDatasetDir.c_str());
        }
        while (!isDirOpened(_dir));
    }

    Status Login(ServerContext* context, const LoginInfo* request, LoginResult* reply) 
    override
    {
        _userId = request->id();
        _userPw = request->pw();

        if (_userId == "juno" && _userPw == "980220")
            reply->set_result(true);
        else
            reply->set_result(false);

        return Status::OK;
    }

    Status DownloadDirEntries (ServerContext* context, const Empty* request, DirEntries* reply) 
    override 
    {
        auto entries = getEntriesFrom(_dir);
        
        for (auto i=0 ; i<entries.size() ; i++)
            reply->add_entries(entries[i]);

        return Status::OK;
    }

    Status DownloadFile(ServerContext* context, const RemoteRequest* request, RemoteReply* reply) // 파일?
    override 
    {
        ifstream ifs;
        string imgName(request->name());
        string imgPath = _pathOfDatasetDir + imgName; // 이미지? 파일? 컨텐츠?
        ifs.open(imgPath, ios::binary);
        string imgBuffer((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());

        reply->set_file(imgBuffer);
        reply->set_name(imgName);
        reply->set_size(imgBuffer.length());
        
        string fileCreationTime;
        struct stat attr;
        if (stat(imgPath.c_str(), &attr) == 0) 
            fileCreationTime = ctime(&attr.st_ctime);

        reply->set_date(fileCreationTime); 
        
        return Status::OK;
    }

private:
    string _userId;
    string _userPw;
    string _pathOfDatasetDir;
    DIR* _dir;
};

class GrpcServiceServer
{
public:
    int RunServer(int argc, char** argv) 
    {
        absl::ParseCommandLine(argc, argv);

        return doRun(absl::GetFlag(FLAGS_port));
    }
protected:
    // ********************************
    // ************ REMARK ************
    // ********************************
    // The contents of doRun() is a reference code 
    // of "greeter_server.cc", from gRPC cpp Hello world example
    virtual int doRun(uint16_t port)
    {
        string serverAddress = absl::StrFormat("0.0.0.0:%d", port);
        grpc::EnableDefaultHealthCheckService(true);
        grpc::reflection::InitProtoReflectionServerBuilderPlugin();

        ServerBuilder builder;
        builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());

        DownloadServer service;
        builder.RegisterService(&service);

        unique_ptr<Server> server(builder.BuildAndStart());
        cout << "Server listening on " << serverAddress << endl;

        server->Wait();

        return 0;
    }
};

int main(int argc, char** argv) 
{
    GrpcServiceServer service;

    return service.RunServer(argc, argv);
}

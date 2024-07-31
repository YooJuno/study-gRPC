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

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using remote::RemoteCommunication;
using remote::RemoteRequest;
using remote::File;
using remote::FileNamesOfDataset;
using remote::Empty;

using namespace std;

ABSL_FLAG(uint16_t, port, 50051, "Server port for the service");

class DirTools
{
public:
    static auto GetFileNamesFrom(DIR* dir) -> vector<string>
    {
        vector<string> result;
        struct dirent* entry;

        while ((entry = readdir(dir)))
        {
            string name = entry->d_name;
            if (name == "." || name == "..") 
                continue;

            result.push_back(name);
        }

        rewinddir(dir);

        return result;
    }

    static auto GetCreationTimeOfFile(const string& path) -> string
    {
        struct stat attr;

        if (stat(path.c_str(), &attr) == 0) 
        {
            string creationTimeOfFile = ctime(&attr.st_ctime);
            *(creationTimeOfFile.end() - 1) = '\0';

            return creationTimeOfFile;
        }
        
        return NULL;
    }
};

class Uploader final : public RemoteCommunication::Service 
{
public:
    Uploader(DIR* dir, const string& pathOfDatasetDir)
    : _dir(dir), _pathOfDatasetDir(pathOfDatasetDir)
    {}

    Status GetFileNamesInDataset(ServerContext* context, const Empty* request, FileNamesOfDataset* reply) override 
    {
        auto fileNames = DirTools::GetFileNamesFrom(_dir);
        
        for (const auto& i : fileNames)
            reply->add_filenames(i);

        return Status::OK;
    }

    Status DownloadFile(ServerContext* context, const RemoteRequest* request, File* reply) override 
    {
        ifstream ifs;
        string targetName(request->name());

        ifs.open(_pathOfDatasetDir + targetName, ios::binary);

        if (!ifs)
        {
            cout << "failed to open file\n";
            reply->set_success(false);
            
            return Status::OK;
        }
        _buffer.assign((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());
    
        ifs.close();

        reply->mutable_header()->set_name(targetName);
        reply->mutable_header()->set_size(_buffer.length());
        reply->mutable_header()->set_date(DirTools::GetCreationTimeOfFile(_pathOfDatasetDir + targetName)); 
        reply->mutable_data()->set_buffer(_buffer);

        return Status::OK;
    }

private:
    string _pathOfDatasetDir;
    DIR* _dir;
    string _buffer;
};

//////////////////////////////////////////////////////////////////////
//                               RENARK                             //
//////////////////////////////////////////////////////////////////////
// void Runserver(uint16_t port, char* pathOfDatasetDir) is         //
// reference code from grpc/example/cpp.                            //
// but *builder.SetMax...Size* codes are                            //
// generated by juno                                                //
//////////////////////////////////////////////////////////////////////
void RunServer(uint16_t port, char* pathOfDatasetDir) 
{
    string serverAddress = absl::StrFormat("0.0.0.0:%d", port);

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    DIR* dir = opendir(pathOfDatasetDir);
    if (!dir)
    {
        cout << "Can't open directory. Please retry.\n\n";\

        return;
    }
    Uploader service(dir, pathOfDatasetDir);

    ServerBuilder builder;

    builder.SetMaxSendMessageSize(1024 * 1024 * 1024 /* == 1GiB */);
    builder.SetMaxReceiveMessageSize(1024 * 1024 * 1024 /* == 1GiB */);

    builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());

    builder.RegisterService(&service);
    
    unique_ptr<Server> server(builder.BuildAndStart());

    cout << "\nServer listening on " << serverAddress << endl;

    server->Wait();
}

int main(int argc, char** argv) 
{
    absl::ParseCommandLine(argc, argv);
    RunServer(absl::GetFlag(FLAGS_port), argv[1]);
    
    return 0;
}

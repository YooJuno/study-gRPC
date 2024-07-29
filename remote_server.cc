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

#include <filesystem>
#include <zip.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerWriter;

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

ABSL_FLAG(uint16_t, port, 50051, "Server port for the service");

class DirTools
{
public:
    static void AddFolderToZip(zip_t* zip, const string& folderPath, const string& zipPath) 
    {
        for (const auto& entry : filesystem::recursive_directory_iterator(folderPath)) 
        {
            if (entry.is_directory()) 
                continue;

            string filePath = entry.path().string();
            string fileNameInZip = zipPath + entry.path().string().substr(folderPath.length() + 1);

            zip_source_t* source = zip_source_file(zip, filePath.c_str(), 0, 0);
            if (source == nullptr) 
            {
                cerr << "Failed to add file to zip: " << filePath << endl;
                continue;
            }

            zip_file_add(zip, fileNameInZip.c_str(), source, ZIP_FL_OVERWRITE);
        }
    }

    static bool ZipFolder(const string& folderPath, const string& zipFilePath)
    {
        int errorp;
        zip_t* zip = zip_open(zipFilePath.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &errorp);
        if (zip == nullptr) 
        {
            zip_error_t error;
            zip_error_init_with_code(&error, errorp);
            zip_error_fini(&error);
            return false;
        }
        
        AddFolderToZip(zip, folderPath, "");
        zip_close(zip);

        return true;
    }

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

    static bool IsDirOpened(DIR* dir)
    {
        return dir;
    }

    static bool IsDir(const string& path)
    {
        return filesystem::is_directory((filesystem::path)path);
    }

    static DIR* OpenDir(string& path)
    {
        DIR* result;
        do
        {
            if (*(path.end() - 1) != '/')
                path += '/';

            result = opendir(path.c_str());

            if (!result)
            {
                cout << "Can't open directory. Please retry.\n\n";\
                return NULL;
            }
        }
        while (!DirTools::IsDirOpened(result));

        return result;
    }
};

class IO
{
public:
    static auto GetDatasetPath() -> string
    {
        string result;

        cout << "Enter dataset path (ex: ../../dataset/)\n: ";
        cin >> result;

        return result;
    }
};

class Uploader final : public RemoteCommunication::Service 
{
public:
    // TODO
    // : 생성자에서 실제 일을 하지 않는다.
    // : 실제 일을 하기 위한 최소한의 리소스 준비
    // (I/O에 대해서는 콜백을 처리하길 권장하심)
    Uploader(DIR* dir)
    : _dir(dir)
    {}

    Status LoginToServer(ServerContext* context, const UserLoginInfo* request, LoginResult* reply) override
    {
        _userId = request->id();
        _userPw = request->pw();

        if (_userId == "juno" && _userPw == "980220") // 추후에 DB연동 시스템으로 확장.
            reply->set_result(true);
        else
            reply->set_result(false);

        return Status::OK;
    }

    Status GetFileNamesOfDataset(ServerContext* context, const Empty* request, FileNamesOfDataset* reply) override 
    {
        auto fileNames = DirTools::GetFileNamesFrom(_dir);
        
        for (const auto& i : fileNames)
            reply->add_filenames(i);

        return Status::OK;
    }

    Status DownloadHeader(ServerContext* context, const RemoteRequest* request, Header* reply) override 
    {
        ifstream ifs;
        string targetName(request->name());
        struct stat attr;

        bool isTargetDir = DirTools::IsDir(_datasetPath + targetName);
        if (isTargetDir)
        {
            bool zipsuccess = DirTools::ZipFolder(_datasetPath + targetName, _datasetPath + targetName + ".zip");

            if (!zipsuccess)
            {
                reply->set_success(false);

                return Status::OK;
            }
            targetName += ".zip";
        }
        
        ifs.open(_datasetPath + targetName, ios::binary);

        if (!ifs)
        {
            cout << "failed to open file\n";
            reply->set_success(false);
            
            return Status::OK;
        }
        _buffer.assign((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());
    
        ifs.close();

        reply->set_name(targetName);
        reply->set_size(_buffer.length());

        if (stat((_datasetPath + targetName).c_str(), &attr) == 0) 
        {
            string creationTimeOfTargetFile = ctime(&attr.st_ctime);
            *(creationTimeOfTargetFile.end() - 1) = '\0';
            reply->set_date(creationTimeOfTargetFile); 
        }
        
        if (isTargetDir)
            filesystem::remove(_datasetPath + targetName);

        return Status::OK;
    }
    
    Status DownloadData(ServerContext* context, const RemoteRequest* request, ServerWriter<Data>* reply) override 
    {
        Data chunk;

        int iter;
        for (iter=0; iter < _buffer.length() - request->chunksize(); iter += request->chunksize())
        {
            chunk.set_buffer(_buffer.substr(iter, request->chunksize()));
            reply->Write(chunk);
        }

        if (iter <  _buffer.length())
        {
            chunk.set_buffer(_buffer.substr(iter, _buffer.length() - iter));
            reply->Write(chunk);
        }

        return Status::OK;
    }

private:
    string _userId;
    string _userPw;
    string _datasetPath;
    DIR* _dir;
    string _buffer;
};

////////////////////////////////////////////
//                 RENARK                 //
////////////////////////////////////////////
// void Runserver(uint16_t port) is       //
// reference code from grpc/example/cpp.  //
// but *builder.SetMax...Size* codes are  //
// generated by juno                      //
////////////////////////////////////////////
void RunServer(uint16_t port) 
{
    string serverAddress = absl::StrFormat("0.0.0.0:%d", port);

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    
    auto datasetPath = IO::GetDatasetPath();
    DIR* dir = DirTools::OpenDir(datasetPath);

    if (dir)
    {
        Uploader service(dir);

        ServerBuilder builder;

        builder.SetMaxSendMessageSize(4 * 1024 * 1024 /* == 4MB */);
        builder.SetMaxMessageSize(4 * 1024 * 1024 /* == 4MB */);
        builder.SetMaxReceiveMessageSize(4 * 1024 * 1024 /* == 4MB */);

        builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());

        builder.RegisterService(&service);
        
        unique_ptr<Server> server(builder.BuildAndStart());
        cout << "\nServer listening on " << serverAddress << endl;

        server->Wait();
    }
    else
    {
        return;
    }   
}

int main(int argc, char** argv) 
{
    absl::ParseCommandLine(argc, argv);
    RunServer(absl::GetFlag(FLAGS_port));
    
    return 0;
}

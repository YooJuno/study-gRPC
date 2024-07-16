/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>


//juno
#include <fstream> 
#include <sys/stat.h>
#include <ctime>
#include <dirent.h>
#define FILE (1)
#define DIRECTORY (2)


#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "jpeg.grpc.pb.h"
#endif

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using jpeg::Greeter;
using jpeg::ImgRequest;
using jpeg::ImgReply;
using jpeg::DirectoryContentsReply;
using jpeg::DirectoryContentsRequest;

using namespace std;

ABSL_FLAG(uint16_t, port, 50051, "Server port for the service");

// Logic and data behind the server's behavior.
// proto 파일에서 정의한 서비스 부분임.
class GreeterServiceImpl final : public Greeter::Service {
    

    bool isDirectory(const string& path)
    {
        struct stat dirStat;

        if (stat(path.c_str(), &dirStat) != 0)
        {
            cout << path << " 경로 정보 알 수 없음" << endl;
            return false;
        }

        return S_ISDIR(dirStat.st_mode);
    }

    bool isFile(const string& path)
    {
        struct stat fileStat;
        if (stat(path.c_str(), &fileStat) != 0)
        {
            cout << path << " 경로 정보 알 수 없음" << endl;
            return false;
        }

        return S_ISREG(fileStat.st_mode);
    }

    vector<string> getDirectoryContents(const string& dirPath, int option = 0)
    {
        vector<string> list;
        DIR* dir = opendir(dirPath.c_str());

        if (dir)
        {
            struct dirent* entry;
            while ((entry = readdir(dir)))
            {
                string name = entry->d_name;

                // 현재 폴더와 상위 폴더는 무시
                if (name == "." || name == "..") continue;
                
                if (option == FILE && isDirectory(dirPath + "/" + name)) continue;
                if (option == DIRECTORY && isFile(dirPath + "/" + name)) continue;

                list.push_back(name);
            }

            closedir(dir);
        }
        else
            cerr << "폴더 열기 실패" << endl;

        return list;
    }

    

    Status SendDirectoryContents(ServerContext* context, 
                    const   DirectoryContentsRequest* request, 
                            DirectoryContentsReply* reply) 
    override {
        id_ = request->id();
        pw_ = request->pw();
        cout << "user ID : " << id_ << endl;
        cout << "user PW : " << pw_ << endl;

        string dataset_path("../../dataset/"); // 현재 경로를 받아와서 경로 탐색이 더 좋을지도
        
        // vector<string> allList = getDirectoryContents(dataset_path);
        vector<string> list;
        DIR* dir = opendir(dataset_path.c_str());
        int cnt = 0;
        if (dir)
        {
            struct dirent* entry;
            while ((entry = readdir(dir)))
            {
                string name = entry->d_name;

                // 현재 폴더와 상위 폴더는 무시
                if (name == "." || name == "..") continue;
                
                if (0 == FILE && isDirectory(dataset_path + "/" + name)) continue;
                if (0 == DIRECTORY && isFile(dataset_path + "/" + name)) continue;

                reply->add_directory(name);
                cnt++;
            }

            closedir(dir);
        }
        else
            cerr << "폴더 열기 실패" << endl;

        // vector<string> fileList = getDirectoryContents(dataset_path, FILE);
        // vector<string> dirList = getDirectoryContents(dataset_path, DIRECTORY);
        
        reply->set_count(cnt);
        
        return Status::OK;
    }


    Status SendImg(ServerContext* context, 
                    const ImgRequest* request, 
                    ImgReply* reply) 
    override {

        cout << '[' << id_ << "] requested " + request->name() << endl;

        ifstream img_in;
        string img_folder_path("../../dataset/");
        string img_name(request->name());
        string img_path = img_folder_path+img_name;
        
        // 이미지의 크기를 byte 단위로 구하기
        img_in.open(img_path, ios::binary);
        string buffer_string((istreambuf_iterator<char>(img_in)), istreambuf_iterator<char>());

        reply->set_size(buffer_string.length());
        reply->set_img(buffer_string);
        reply->set_name(request->name());
        
        string file_creation_time;
        struct stat attr;

        if (stat(img_path.c_str(), &attr) == 0) 
            file_creation_time = ctime(&attr.st_ctime);

        reply->set_date(file_creation_time); 
        
        return Status::OK;
    }
    private:
    string id_;
    string pw_;
    
};

void RunServer(uint16_t port) {
    string server_address = absl::StrFormat("0.0.0.0:%d", port);

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    // Listen on the given address without any authentication mechanism.
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    GreeterServiceImpl service;
    builder.RegisterService(&service);

    // Finally assemble the server.
    unique_ptr<Server> server(builder.BuildAndStart());
    cout << "Server listening on " << server_address << endl;

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();
}

int main(int argc, char** argv) {
    absl::ParseCommandLine(argc, argv);
    RunServer(absl::GetFlag(FLAGS_port));
    return 0;
}

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

#include <fstream> 
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include <grpcpp/grpcpp.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "jpeg.grpc.pb.h"
#endif

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using jpeg::Patcher;

using jpeg::ImgRequest;
using jpeg::ImgReply;

using jpeg::LoginInfo;
using jpeg::LoginResult;

using jpeg::DirContents;

using namespace std;
ABSL_FLAG(string, target, "localhost:50051", "Server address");

class PatcherClient 
{
private:
    unique_ptr<Patcher::Stub> stub_;

public:
    PatcherClient(shared_ptr<Channel> channel)
        : stub_(Patcher::NewStub(channel)) {}

    // string DirInfo()
    // {   
    //     DirContents reply; // request가 필요 없을 때는 어떻게 해야되지?
    //     ClientContext context;

    //     Status status = stub_->PrintDirInfo(&context, reply, &reply);

    //     vector<string> Dir_info;
    //     if (status.ok()) 
    //     {
    //         cout << "\n[아래의 목록 중에 선택해주세요]\n";
    //         for(int i=0 ; i<reply.count() ; i++)
    //         {
    //             cout << i+1 << ". " << reply.entries(i) << endl;
    //             Dir_info.push_back(reply.entries(i));
    //         }
    //     } 
    //     else 
    //     {
    //         cout << status.error_code() << ": " << status.error_message() << endl;
    //         exit(-1);
    //     }

    //     cout << "\n번호를 입력하세요 : " ;
    //     int num;
    //     cin>>num;
    //     cout << "\n";

    //     return Dir_info[num-1];
    // }

    vector<string> LoginToServ(const string& id, const string& pw)
    {
        vector<string> output;
        LoginInfo request;
        LoginResult reply;

        request.set_id(id);
        request.set_pw(pw);

        ClientContext context;

        Status status = stub_->Login(&context, request, &reply);


        
        if (status.ok()) 
        {
            if(reply.result() == "Success")
            {

            }
            return ;
        } 
        else 
        {   
            output.push_back(status.error_code() + ": " + status.error_message());
            return output;
        }
    }
    
    void DownloadFile(const string& file_name) 
    {
        ImgRequest request; // Data we are sending to the server.
        ImgReply reply; // Container for the data we expect from the server.
        
        request.set_name(file_name);

        ClientContext context;

        // The actual RPC. Send data
        Status status = stub_->DownloadImg(&context, request, &reply); 

        // Act upon its status.
        if (status.ok()) 
        {
            string file_name  (reply.name()); 
            int         file_size = reply.size(); 
            string file_img  = reply.img(); 
            string file_date  (reply.date()); 
            
            cout<<"file name : "+file_name<<endl;
            cout<<"size : "<<file_size<<" Bytes"<<endl;
            cout<<"date : "+file_date<<endl;
            
            ofstream ofs;
            ofs.open("../../dataset/" + file_name, ios::out | ios::binary);
            ofs.write((char *)file_img.data(), file_img.length());           
            ofs.close();
        } 
        else 
            cout << status.error_code() << ": " << status.error_message() << endl;
    }
};

int main(int argc, char** argv) 
{
    absl::ParseCommandLine(argc, argv);

    string target_str = absl::GetFlag(FLAGS_target);
    
    string ID;
    string PW;
    cout << "ID : ";
    cin >> ID;
    cout << "PW : ";
    cin >> PW;

    PatcherClient patcher(grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
    
    vector<string> FileEntriesInServ = patcher.LoginToServ(ID,PW);

    // string replyDirectory = patcher.DirInfo();

    patcher.DownloadFile("temp");

    return 0;
}


// RemoteFilePatcher pather();
// auto dirEntries = patcher.GetDirEntiries(".");
// auto file = dirEntries[0].IsFile ? dirEntries[0].Path : "";
// auto fileContent = patcher.GetFile(file);
// ofstreawm ofs(path);
// ofs.write(fileContent);

// referential integrity of
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

using jpeg::RequestFile;
using jpeg::ReplyFile;

using jpeg::LoginInfo;
using jpeg::LoginResult;

using jpeg::Permission;
using jpeg::ReplyJobList;

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

    bool TryLoginToServ()
    {   
        // cout << "TryLoginToServ\n" ;
        vector<string> login_info = InputLoginInfo();
        
        string login_result = Login(login_info[0],login_info[1]);

        int try_cnt=0;

        while(login_result == "Login Fail")
        {
            if(try_cnt>=3)
            {
                cout<<"3회 이상 틀리셨습니다. 이용을 종료합니다\n";
                return false;
            }

            login_info = InputLoginInfo();
            login_result = Login(login_info[0],login_info[1]);
            cout << login_result << endl;
            try_cnt++;
        }

        return true;
    }

    vector<string> InputLoginInfo()
    {   
        string ID;
        string PW;
        cout << "ID : ";
        cin >> ID;
        cout << "PW : ";
        cin >> PW;

        vector<string> output = {ID, PW};

        return output;
    }

    string Login(const string& id, const string& pw)
    {
        // cout << "Login\n" ;
        LoginInfo request;
        LoginResult reply;

        request.set_id(id);
        request.set_pw(pw);

        ClientContext context;

        Status status = stub_->Login(&context, request, &reply);
        
        string output;

        if (status.ok()) 
            output = reply.result();
        else 
            output = status.error_code() + ": " + status.error_message();
        
        cout << "[login_result] " << output << endl;

        return output;
    }


    vector<string> PatchJobList(bool permission)
    {   
        // cout << "PatchJobList\n" ;
        Permission request; // Data we are sending to the server.
        ReplyJobList reply; // Container for the data we expect from the server.
        ClientContext context;

        request.set_permission(permission); // 함수 인자로 받아서 넣어야됨

        Status status = stub_->PatchJobList(&context, request, &reply); 

        vector<string> output;

        if (status.ok()) 
        {
            for(int i=0 ; i<reply.v_size() ; i++)
            { 
                output.push_back(reply.list(i));
            }
        }
        else 
        {
            cout << status.error_code() << ": " << status.error_message() << endl;
            exit(-1);
        }
        
        return output;
    }
    
    void PatchFile(const string& file_name) 
    {
        // cout << "PatchFile\n" ;
        RequestFile request; // Data we are sending to the server.
        ReplyFile reply; // Container for the data we expect from the server.
        
        request.set_name(file_name);

        ClientContext context;

        // The actual RPC. Send data
        Status status = stub_->PatchFile(&context, request, &reply); 

        // Act upon its status.
        if (status.ok()) 
        {
            string file_name  (reply.name()); 
            int file_size = reply.size(); 
            string file_img  = reply.img(); 
            string file_date  (reply.date()); 
            
            cout << "file name : "+file_name << endl;
            cout << "size : " << file_size << " Bytes" << endl;
            cout << "date : "+file_date << endl;
            
            ofstream ofs;
            ofs.open("../../download/" + file_name, ios::out | ios::binary);
            ofs.write((char *)file_img.data(), file_img.length());           
            ofs.close();
        } 
        else 
            cout << status.error_code() << ": " << status.error_message() << endl;
    }
    
    string DirInfo(bool permission)
    {   
        // cout << "DirInfo\n" ;
        Permission request;
        DirContents reply;

        ClientContext context;

        request.set_permission(permission);

        Status status = stub_->PrintDirInfo(&context, request, &reply);

        vector<string> Dir_info;
        if (status.ok())
        {
            cout << "\n[Choose Image you wanna download]\n";
            for(int i=0 ; i<reply.v_size() ; i++)
            {
                cout << i+1 << ". " << reply.list(i) << endl;
                Dir_info.push_back(reply.list(i));
            }
        } 
        else
        {
            cout << status.error_code() << ": " << status.error_message() << endl;
            exit(-1);
        }

        cout << "\n번호를 입력하세요 : " ;
        int num;
        cin>>num;
        cout << "\n";

        return Dir_info[num-1];
    }
};

int main(int argc, char** argv) 
{
    absl::ParseCommandLine(argc, argv);
    string target_str = absl::GetFlag(FLAGS_target);
    PatcherClient patcher(grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
    
    bool permission = false;

    if(patcher.TryLoginToServ() == false) 
        return 0;
    else 
        permission = true;
    
    vector<string> job_list = patcher.PatchJobList(permission);

    cout << "\n[Choose job you wanna do]\n";
    for(int i=0 ; i<job_list.size() ; i++){
        cout << "[" << i+1 << "] " << job_list[i] << endl;
    }
    cout << "Input : " ;
    int num;
    cin >> num;

    string file_name = patcher.DirInfo(permission);
    
    patcher.PatchFile(file_name);
    
    return 0;
}


// RemoteFilePatcher pather();
// auto dirEntries = patcher.GetDirEntiries(".");
// auto file = dirEntries[0].IsFile ? dirEntries[0].Path : "";
// auto fileContent = patcher.GetFile(file);
// ofstreawm ofs(path);
// ofs.write(fileContent);

// referential integrity of
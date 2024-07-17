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
#include <tuple>

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

using jpeg::Downloader;

using jpeg::RequestFile;
using jpeg::ReplyFile;

using jpeg::LoginInfo;
using jpeg::LoginResult;

using jpeg::ReplyJobEntries;

using jpeg::DirEntries;

using jpeg::Empty;

using namespace std;

ABSL_FLAG(string, target, "localhost:50051", "Server address");

class DownloaderClient 
{
public:
    DownloaderClient(shared_ptr<Channel> channel)
        : _stub (Downloader::NewStub(channel)) {}

    //
    auto InputLoginInfoByUser() -> std::pair<string, string> 
    {   
        string id;
        string pw;
        cout << "ID : ";
        cin >> id;
        cout << "PW : ";
        cin >> pw;

        return make_pair(id, pw);
    }

    bool TryLoginToServer(const string& id, const string& pw)
    {
        ClientContext context;
        LoginInfo request;
        LoginResult reply;

        request.set_id(id);
        request.set_pw(pw);

        Status status = _stub->Login(&context, request, &reply);

        if (status.ok())
        {
            return reply.result();
        }
        else
        {
            cout << status.error_code() + ": " + status.error_message() << endl;
            return false;
        }
    }

    auto DownloadJobEntries() -> vector<string> 
    {   
        Empty request; // Data we are sending to the server.
        ReplyJobEntries reply; // Container for the data we expect from the server.
        ClientContext context;

        Status status = _stub->DownloadJobEntries(&context, request, &reply); 

        vector<string> output;

        if (status.ok()) 
        {
            for (int i=0; i<reply.v_size(); i++)
                output.push_back(reply.entries(i));
        }
        else 
        {
            cout << status.error_code() << ": " << status.error_message() << endl;
            exit(-1);
        }
        
        return output;
    }
    
    void Download(const string& fileName)  
    {
        ClientContext context;
        RequestFile request; // Data we are sending to the server.
        ReplyFile reply; // Container for the data we expect from the server.
        
        request.set_name(fileName);

        // The actual RPC. Send data
        Status status = _stub->DownloadFile(&context, request, &reply); 

        // Act upon its status.
        if (status.ok()) 
        {
            string  fileImg =  reply.img(); 
            
            cout << "\nfile name : "+reply.name() << endl;
            cout << "size : " << reply.size() << " Bytes" << endl;
            cout << "date : "+reply.date() << endl;
            
            ofstream ofs;
            ofs.open("../../download/" + fileName, ios::out | ios::binary);
            ofs.write((char *)fileImg.data(), fileImg.length());           
            ofs.close();
        } 
        else 
        {
            cout << status.error_code() << ": " << status.error_message() << endl;
        }
    }
    
    auto DownloadDirEntries() -> vector<string> 
    {   
        vector<string> output;

        ClientContext context;
        Empty request;
        DirEntries reply;

        Status status = _stub->DownloadDirEntries(&context, request, &reply);

        if (status.ok())
        {
            for(int i=0; i<reply.v_size(); i++)
                output.push_back(reply.entries(i));
        }
        else
        {
            cout << status.error_code() << ": " << status.error_message() << endl;
            exit(-1);
        }

        return output;
    }

private:
    unique_ptr<Downloader::Stub> _stub;
};

string chooseFrom(vector<string> list)
{
    for (int i=0; i<list.size(); i++)
    {
        cout << "[" << i+1 << "] " << list[i] << endl;
    }

    cout << "Input num: " ;
    int num;
    cin >> num;

    return list[num-1];
}

// Application Framework
// ToolKit

class GrpcService
{
public:
    int RunClient(int argc, char** argv)
    {
        absl::ParseCommandLine(argc, argv);
        string target_str = absl::GetFlag(FLAGS_target);
        
        return doRun(target_str);
    }

protected:
    virtual int doRun(string target_str)
    {
        DownloaderClient Downloader(grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
        string userId;
        string userPw;
        std::tie(userId, userPw) = Downloader.InputLoginInfoByUser();

        auto permission = Downloader.TryLoginToServer(userId, userPw);

        int errCnt =0;
        
        while (!permission)
        {
            errCnt++;

            if (errCnt > 3)
            {
                cout << "3회 이상 시도하였습니다. 이용을 종료합니다\n";
                return 0;
            }

            cout << "Incorrect ID or PW. Please retry\n";

            std::tie(userId, userPw) = Downloader.InputLoginInfoByUser();
            permission = Downloader.TryLoginToServer(userId, userPw);
        }

        if (permission)
        {
            auto jobList = Downloader.DownloadJobEntries();
            cout << "\n[Choose what you wanna do]\n";
            auto jobName = chooseFrom(jobList);

            if (jobName == "Download")
            {
                auto datasetEntries = Downloader.DownloadDirEntries();
                cout << "\n[Choose what you wanna download]\n";
                auto fileName = chooseFrom(datasetEntries);
                
                Downloader.Download(fileName);
            }
            else if (jobName == "Upload")
            {
                return 0;
            }
            else
            {
                return 0;
            }
        }

        return 0;
    }

};

class FileBasedGrpcService: public GrpcService 
{
protected:
    virtual int doRun(string target_str) 
    {
        return 0;
    }
};

int main(int argc, char** argv)  
{
    GrpcService service_org;
    FileBasedGrpcService service;

    return service_org.RunClient(argc, argv);
}

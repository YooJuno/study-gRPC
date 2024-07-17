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

using jpeg::ReplyJobEntries;

using jpeg::DirEntries;

using jpeg::Empty;

using namespace std;

ABSL_FLAG(string, target, "localhost:50051", "Server address");

class PatcherClient 
{
private:
    unique_ptr<Patcher::Stub> _stub;

public:
    PatcherClient(shared_ptr<Channel> channel)
        : _stub (Patcher::NewStub(channel)) {}

    void InputLoginInfoByUser (string& id, string& pw)
    {   
        cout << "ID : ";
        cin >> id;
        cout << "PW : ";
        cin >> pw;
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

    vector<string> PatchJobEntries()
    {   
        Empty request; // Data we are sending to the server.
        ReplyJobEntries reply; // Container for the data we expect from the server.
        ClientContext context;

        Status status = _stub->PatchJobEntries(&context, request, &reply); 

        vector<string> output;

        if (status.ok()) 
        {
            for (int i=0; i<reply.v_size(); i++)
            { 
                output.push_back(reply.entries(i));
            }
        }
        else 
        {
            cout << status.error_code() << ": " << status.error_message() << endl;
            exit(-1);
        }
        
        return output;
    }
    
    void Patch(const string& fileName)  
    {
        ClientContext context;
        RequestFile request; // Data we are sending to the server.
        ReplyFile reply; // Container for the data we expect from the server.
        
        request.set_name(fileName);

        // The actual RPC. Send data
        Status status = _stub->PatchFile(&context, request, &reply); 

        // Act upon its status.
        if (status.ok()) 
        {
            string  fileImg =  reply.img(); 
            
            cout << "file name : "+reply.name() << endl;
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
    
    vector<string> PatchDatasetEntries()
    {   
        vector<string> output;

        ClientContext context;
        Empty request;
        DirEntries reply;

        Status status = _stub->PatchDirInfo(&context, request, &reply);

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
    
    cout << "\n";

    return list[num-1];
}

class GrpcServiceClient
{
public:
    int StartService(int argc, char** argv)
    {
        absl::ParseCommandLine(argc, argv);
        string target_str = absl::GetFlag(FLAGS_target);
        PatcherClient patcher(grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));

        string userID;
        string userPW;
        patcher.InputLoginInfoByUser(userID, userPW);

        auto permission = patcher.TryLoginToServer(userID, userPW);

        int errCnt=0;
        
        while (!permission)
        {
            errCnt++;

            if (errCnt>3)
            {
                cout << "3회 이상 시도하였습니다. 이용을 종료합니다\n";
                return 0;
            }

            cout << "Incorrect ID or PW. Please retry\n";

            patcher.InputLoginInfoByUser(userID, userPW);
            permission = patcher.TryLoginToServer(userID, userPW);
        }

        if (permission)
        {
            auto jobList = patcher.PatchJobEntries();
            cout << "\n[Choose what you wanna do]\n";
            auto jobName = chooseFrom(jobList);

            if (jobName == "Download")
            {
                auto datasetEntries = patcher.PatchDatasetEntries();
                cout << "\n[Choose what you wanna download]\n";
                auto fileName = chooseFrom(datasetEntries);
                
                patcher.Patch(fileName);
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

int main(int argc, char** argv)  
{
    GrpcServiceClient grpcClient;
    return grpcClient.StartService(argc, argv);
}


// RemoteFilePatcher pather();
// auto dirEntries = patcher.GetDirEntiries(".");
// auto file = dirEntries[0].IsFile ? dirEntries[0].Path : "";
// auto fileContent = patcher.GetFile(file);
// ofstreawm ofs(path);
// ofs.write(fileContent);

// referential integrity of

// [07/17]
// class GrpcPictureService
// {
// public:
//     int StartService()
//     {
//         return 0;
//     }

// };
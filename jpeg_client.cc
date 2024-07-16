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

//juno
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

using jpeg::Greeter;

using jpeg::ImgRequest;
using jpeg::ImgReply;
using jpeg::DirectoryContentsRequest;
using jpeg::DirectoryContentsReply;

using namespace std;
ABSL_FLAG(string, target, "localhost:50051", "Server address");

class GreeterClient {
    public:
    GreeterClient(shared_ptr<Channel> channel)
        : stub_(Greeter::NewStub(channel)) {}

    // Assembles the client's payload, sends it and presents the response back
    // from the server.
    void ImgFile(const string& file_name) {
            
        // Data we are sending to the server.
        ImgRequest request;
        // Container for the data we expect from the server.
        ImgReply reply;
        
        request.set_name(file_name);

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        ClientContext context;

        // The actual RPC.
        Status status = stub_->SendImg(&context, request, &reply); //SEND

        // Act upon its status.
        if (status.ok()) {
            string file_name  (reply.name()); 
            int         file_size = reply.size(); 
            string file_img  = reply.img(); 
            string file_date  (reply.date()); 
            
            cout<<"file name : "+file_name<<endl;
            cout<<"size : "<<file_size<<"Bytes"<<endl;
            cout<<"date : "+file_date<<endl;
            
            ofstream img_out;
            img_out.open("../../dataset/download_" + file_name, ios::out | ios::binary);

            // [성공]
            // img_out << file_img;
            
            // [실패]
            // img_out.write((char *)&file_img, file_size);

            // [성공]
            for(int i=0 ; i<file_size ; i++){
                img_out << (char)file_img[i];
            }
            img_out.close();
        } 
        else {
            cout << status.error_code() << ": " << status.error_message() << endl;
        }
    }

    string DirectoryInfo(const string& id, const string& pw)
    {
        DirectoryContentsRequest request_d;
        DirectoryContentsReply reply_d;
        
        request_d.set_id(id);
        request_d.set_pw(pw);

        ClientContext context;

        Status status = stub_->SendDirectoryContents(&context, request_d, &reply_d); //SEND
        vector<string> directory_info;
        if (status.ok()) {
            
            cout << "\n[아래의 목록 중에 선택해주세요]\n";
            for(int i=0 ; i<reply_d.count() ; i++){
                cout << i+1 << ". " << reply_d.directory(i) << endl;
                directory_info.push_back(reply_d.directory(i));
            }

            
        } 
        else {
            cout << status.error_code() << ": " << status.error_message() << endl;
            exit(-1);
        }
        cout << "\n";

        cout << "번호를 입력하세요 : " ;
        int num;
        cin>>num;
        cout << "\n";
        return directory_info[num-1];
    }

    private:
    unique_ptr<Greeter::Stub> stub_;
    string id_;
    string pw_;
};

int main(int argc, char** argv) {
    absl::ParseCommandLine(argc, argv);
    // Instantiate the client. It requires a channel, out of which the actual RPCs
    // are created. This channel models a connection to an endpoint specified by
    // the argument "--target=" which is the only expected argument.
    string target_str = absl::GetFlag(FLAGS_target);

    // We indicate that the channel isn't authenticated (use of
    // InsecureChannelCredentials()).
    GreeterClient greeter(
        grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
    
    string ID;
    string PW;
    cout << "ID : ";
    cin >> ID;
    cout << "PW : ";
    cin >> PW;

    string reply_directory = greeter.DirectoryInfo(ID, PW);

    
    greeter.ImgFile(reply_directory);

    return 0;
}


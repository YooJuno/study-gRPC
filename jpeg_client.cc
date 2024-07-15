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
#include <fstream> //juno

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include <grpcpp/grpcpp.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "jpeg.grpc.pb.h"
#endif

ABSL_FLAG(std::string, target, "localhost:50051", "Server address");

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using jpeg::Greeter;

using jpeg::HelloReply;
using jpeg::HelloRequest;

using jpeg::ImgRequest;
using jpeg::ImgReply;

class GreeterClient {
 public:
  GreeterClient(std::shared_ptr<Channel> channel)
      : stub_(Greeter::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  // std::string SayHello(const std::string& user) { juno
  std::string SendImg(const std::string& user) {
    
    // Data we are sending to the server.
    // HelloRequest request;
    // request.set_name(user);

    ImgRequest request;
    std::ifstream readFile;
    std::string path = "../../dataset/image.jpeg";
    readFile.open(path);

    // if(readFile.is_open()){   //파일이 열렸는지 확인
    //   while(!readFile.eof()){   //파일 끝까지 읽었는지 확인
    //     char arr[];
    //     readFile.getline(arr, 256);   //한줄씩 읽어오기
    //   }
    // } 한 줄 씩 읽어들이면서 string 뒤에 추가하고, 스트링은 한 번에 보내면 될 듯
    // 크기도 불러올 때 마다 스트링에 넣어주고.

    char temp[100000];
    readFile.read(temp, 1000000);
    std::string buf = temp;
    request.set_img(buf);
    request.set_name("juno");
    request.set_date("20240715");

    // Container for the data we expect from the server.
    // HelloReply reply;
    ImgReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->SendImg(&context, request, &reply); //SEND

    // Act upon its status.
    if (status.ok()) {
      return reply.name();
    } else {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return "RPC failed";
    }
  }



 private:
  std::unique_ptr<Greeter::Stub> stub_;
};

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint specified by
  // the argument "--target=" which is the only expected argument.
  std::string target_str = absl::GetFlag(FLAGS_target);
  // We indicate that the channel isn't authenticated (use of
  // InsecureChannelCredentials()).
  GreeterClient greeter(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
  std::string user = "juno1";
  std::string reply = greeter.SendImg(user);
  std::cout << "Greeter received: " << reply << std::endl;

  return 0;
}


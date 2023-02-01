#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "messenger.grpc.pb.h"

using google::protobuf::Empty;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using messenger::Message;
using messenger::Messenger;

class MessengerServiceImpl final : public Messenger::Service {
 public:
  Status LogIn([[maybe_unused]] ServerContext* context,
               const messenger::LogInRequest* request,
               [[maybe_unused]] Empty* response) override {
    std::cout << "LogIn request\nLogin: " << request->login()
              << "\nPassword: " << request->password() << "\n";
    if (!request->login().empty() && !request->password().empty()) {
      std::cout << "Valid credentials\n";
      return Status::OK;
    }
    std::cout << "Invalid credentials\n";
    Status status(grpc::UNAUTHENTICATED, "Invalid login/password");
    return status;
  }

  Status LogOut([[maybe_unused]] ServerContext* context,
                const messenger::LogOutRequest* request,
                [[maybe_unused]] Empty* responce) override {
    std::cout << "LogOut request:\nLogin: " << request->login() << '\n';
    return Status::OK;
  }

  Status OpenMessageStream(
      [[maybe_unused]] ServerContext* context,
      grpc::ServerReaderWriter<Message, Message>* stream) override {
    std::cout << "Message stream opened.\n";
    auto read_thread = std::jthread([stream] {
      Message msg;
      while (stream->Read(&msg)) {
        std::cout << "New message:\nSender: " << msg.sender()
                  << "\nReceiver: " << msg.receiver()
                  << "\nData: " << msg.data() << '\n';
      }
    });
    return Status::OK;
  }
};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  MessengerServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << '\n';

  server->Wait();
}

int main() {
  RunServer();
  return 0;
}

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
using grpc::ServerReader;
using grpc::ServerWriter;
using grpc::Status;
using messenger::Message;
using messenger::Messenger;
using messenger::ValidationRequest;

class MessengerServiceImpl final : public Messenger::Service {
 public:
  Status Authenticate(ServerContext* context, const ValidationRequest* request,
                      Empty* response) override {
    if (!request->login().empty() && !request->password().empty()) {
      return Status::OK;
    }
    Status status(grpc::UNAUTHENTICATED, "Invalid login/password");
    return status;
  }

  Status SendMessages(ServerContext* context, ServerReader<Message>* reader,
                      Empty* response) override {
    std::jthread reader_thread([reader] {
      Message message;
      std::cout << "Server: Opened message stream (thread "
                << std::this_thread::get_id() << ")" << std::endl;
      while (reader->Read(&message)) {
        std::cout << "Sender: " << message.sender()
                  << "\nReceiver: " << message.receiver() << "\n\t"
                  << message.data() << std::endl;
      }
      std::cout << "Server: Message stream closed" << std::endl;
    });

    return Status::OK;
  }

  Status AcceptMessages(ServerContext* context, const Empty* request,
                        ServerWriter<Message>* writer) override {
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
  std::cout << "Server listening on " << server_address << std::endl;

  server->Wait();
}

int main() {
  RunServer();
  return 0;
}

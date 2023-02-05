#include <iostream>

#include "server_implementation.h"

void ServerControl(grpc::Server& server) {
  int c = 0;
  while ((c = getchar()) == 0) {
    std::this_thread::yield();
  }
  server.Shutdown();
}

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  MessengerServiceImpl service;
  std::cout << "Server created\n";

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  service.LoadData();

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << '\n';

  std::jthread input_thread{ServerControl, std::ref(*server)};

  server->Wait();
}

int main() {
  RunServer();
  return 0;
}

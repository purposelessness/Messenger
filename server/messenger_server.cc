#include <iostream>

#include "server_implementation.h"

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  MessengerServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << '\n';

  server->Wait();
}

int main() {
  RunServer();
  return 0;
}

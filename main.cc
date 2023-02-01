#include <grpcpp/grpcpp.h>

#include <thread>

#include "interface/interface_service.h"
#include "net/net_service.h"

int main() {
  std::string target_str("localhost:50051");
  InterfaceService interface_service;
  NetService net_service(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));

  auto& interface_bus = interface_service.GetBus();
  auto& net_bus = net_service.GetBus();
  interface_service.SetOutputBus(&net_bus);
  net_service.SetOutputBus(&interface_bus);

  std::jthread interface_thread(&InterfaceService::Run, &interface_service);
  std::jthread net_thread(&NetService::Run, &net_service);

  return 0;
}

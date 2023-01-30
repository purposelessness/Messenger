#include <thread>

#include "interface/interface_service.h"
#include "net/net_service.h"

int main() {
  InterfaceService interface_service;
  NetService net_service;

  auto& interface_bus = interface_service.GetBus();
  auto& net_bus = net_service.GetBus();
  interface_service.SetOutputBus(&net_bus);
  net_service.SetOutputBus(&interface_bus);

  std::thread interface_thread(&InterfaceService::Run, &interface_service);
  std::thread net_thread(&NetService::Run, &net_service);

  interface_thread.join();
  net_thread.join();
  return 0;
}

#include <iostream>
#include <thread>

#include "Interface/InterfaceService.h"
#include "Net/NetService.h"

int main() {
  InterfaceService interface_service;
  NetService net_service;

  auto& interface_bus = interface_service.getBus();
  auto& net_bus = net_service.getBus();
  interface_service.setOutputBus(&net_bus);
  net_service.setOutputBus(&interface_bus);

  std::thread interface_thread(&InterfaceService::run, &interface_service);
  std::thread net_thread(&NetService::run, &net_service);

  interface_thread.join();
  net_thread.join();
  return 0;
}

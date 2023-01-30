#include "NetService.h"

#include <iostream>

void NetService::run() {
  _is_active = true;
  while (_is_active) {
    if (_bus.empty()) {
      std::this_thread::yield();
    } else {
      _bus.waitAndPop()->accept(*this);
    }
  }
}

void NetService::visit(const LoginMessage& message) {
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "Server new login:\n" << message.login << '\n';
  if (_out_bus != nullptr) {
    _out_bus->push(
        std::make_unique<LoginMessage>(message.login, message.password));
  }
}

void NetService::visit(const ChatMessage& message) {
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "Server new message:\n" << message.data << '\n';
  if (_out_bus != nullptr) {
    _out_bus->push(std::make_unique<ChatMessage>(message.data));
  }
}

void NetService::visit(const QuitMessage& message) {
  std::cout << "Net service is shutting down\n";
  _is_active = false;
}

void NetService::setOutputBus(NetService::Bus* out_bus) { _out_bus = out_bus; }

NetService::Bus& NetService::getBus() { return _bus; }

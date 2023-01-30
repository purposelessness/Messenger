#include "net_service.h"

#include <iostream>

void NetService::Run() {
  is_active_ = true;
  while (is_active_) {
    if (bus_.Empty()) {
      std::this_thread::yield();
    } else {
      bus_.WaitAndPop()->Accept(*this);
    }
  }
}

void NetService::Visit(const LoginMessage& message) {
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "Server new Login:\n" << message.login << '\n';
  if (out_bus_ != nullptr) {
    out_bus_->Push(
        std::make_unique<LoginMessage>(message.login, message.password));
  }
}

void NetService::Visit(const ChatMessage& message) {
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "Server new message:\n" << message.data << '\n';
  if (out_bus_ != nullptr) {
    out_bus_->Push(std::make_unique<ChatMessage>(message.data));
  }
}

void NetService::Visit(const QuitMessage& message) {
  std::cout << "Net service is shutting down\n";
  is_active_ = false;
}

void NetService::SetOutputBus(NetService::Bus* out_bus) { out_bus_ = out_bus; }

NetService::Bus& NetService::GetBus() { return bus_; }

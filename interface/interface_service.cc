#include "interface_service.h"

#include <iostream>

InterfaceService::InterfaceService()
    : command_map_({{'0', [this] { Quit(); }},
                    {'1', [this] { Login(); }},
                    {'2', [this] { EnterMessage(); }}}) {}

void InterfaceService::Run() {
  PrintInfo();

  is_active_ = true;
  while (is_active_) {
    if (bus_.Empty()) {
      Process();
      std::this_thread::yield();
    } else {
      bus_.WaitAndPop()->Accept(*this);
    }
  }
}

void InterfaceService::PrintInfo() {
  std::cout << "0: Quit\n1: Login\n2: enter message\n";
}

void InterfaceService::Process() {
  char c = '0';
  std::cin >> c;
  if (!command_map_.contains(c)) {
    std::cout << "Wrong command\n";
    return;
  }
  command_map_[c]();
}

void InterfaceService::Quit() {
  std::cout << "Interface is shutting down\n";
  out_bus_->Push(std::make_unique<QuitMessage>());
  is_active_ = false;
}

void InterfaceService::Login() {
  std::string login;
  std::string password;
  std::cout << "Enter Login:\n";
  std::cin >> login;
  std::cout << "Enter password:\n";
  std::cin >> password;
  out_bus_->Push(
      std::make_unique<LoginMessage>(std::move(login), std::move(password)));
}

void InterfaceService::EnterMessage() {
  std::string message;
  std::cout << "Enter message:\n";
  std::cin >> message;
  out_bus_->Push(std::make_unique<ChatMessage>(std::move(message)));
}

void InterfaceService::Visit(const LoginMessage& message) {
  std::cout << "Logged in!\nLogin: " << message.login
            << "\nPassword: " << message.password << '\n';
}

void InterfaceService::Visit(const ChatMessage& message) {
  std::cout << "New message:\n" << message.data << '\n';
}

void InterfaceService::Visit([[maybe_unused]] const QuitMessage& message) {
  Quit();
}

void InterfaceService::SetOutputBus(Bus* out_bus) { out_bus_ = out_bus; }

InterfaceService::Bus& InterfaceService::GetBus() { return bus_; }

bool InterfaceService::IsActive() const { return is_active_; }

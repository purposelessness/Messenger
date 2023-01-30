#include "InterfaceService.h"

#include <iostream>

InterfaceService::InterfaceService()
    : _command_map({{'0', [this] { quit(); }},
                    {'1', [this] { login(); }},
                    {'2', [this] { enterMessage(); }}}) {}

void InterfaceService::run() {
  printInfo();

  _is_active = true;
  while (_is_active) {
    if (_bus.empty()) {
      process();
      std::this_thread::yield();
    } else {
      _bus.waitAndPop()->accept(*this);
    }
  }
}

void InterfaceService::printInfo() {
  std::cout << "0: quit\n1: login\n2: enter message\n";
}

void InterfaceService::process() {
  char c = '0';
  std::cin >> c;
  if (!_command_map.contains(c)) {
    std::cout << "Wrong command\n";
    return;
  }
  _command_map[c]();
}

void InterfaceService::quit() {
  std::cout << "Interface is shutting down\n";
  _out_bus->push(std::make_unique<QuitMessage>());
  _is_active = false;
}

void InterfaceService::login() {
  std::string login;
  std::string password;
  std::cout << "Enter login:\n";
  std::cin >> login;
  std::cout << "Enter password:\n";
  std::cin >> password;
  _out_bus->push(
      std::make_unique<LoginMessage>(std::move(login), std::move(password)));
}

void InterfaceService::enterMessage() {
  std::string message;
  std::cout << "Enter message:\n";
  std::cin >> message;
  _out_bus->push(std::make_unique<ChatMessage>(std::move(message)));
}

void InterfaceService::visit(const LoginMessage& message) {
  std::cout << "Logged in!\nLogin: " << message.login
            << "\nPassword: " << message.password << '\n';
}

void InterfaceService::visit(const ChatMessage& message) {
  std::cout << "New message:\n" << message.data << '\n';
}

void InterfaceService::visit(const QuitMessage& message) { quit(); }

void InterfaceService::setOutputBus(Bus* out_bus) { _out_bus = out_bus; }

InterfaceService::Bus& InterfaceService::getBus() { return _bus; }

bool InterfaceService::isActive() const { return _is_active; }

#include "net_service.h"

#include <iostream>

NetService::NetService(const std::shared_ptr<grpc::Channel>& channel)
    : stub_(messenger::Messenger::NewStub(channel)) {}

void NetService::Run() {
  is_active_ = true;
  grpc::ClientContext context;
  stream_ = stub_->OpenMessageStream(&context);
  while (is_active_) {
    if (bus_.Empty()) {
      std::this_thread::yield();
    } else {
      bus_.WaitAndPop()->Accept(*this);
    }
  }
  stream_->WritesDone();
  auto status = stream_->Finish();
  if (!status.ok()) {
    std::cout << "Net service: chat ended w/ status code "
              << status.error_code() << " and message \""
              << status.error_message() << "\".\n";
  }
  LogOut();
}

void NetService::LogOut() {
  messenger::LogOutRequest request;
  request.set_login("Me");
  google::protobuf::Empty empty;

  grpc::ClientContext context;
  auto status = stub_->LogOut(&context, request, &empty);
  std::cout << "User logged out\n";
}

void NetService::Visit(const LoginMessage& message) {
  messenger::LogInRequest request;
  request.set_login(message.login);
  request.set_password(message.password);
  google::protobuf::Empty empty;

  grpc::ClientContext context;
  grpc::Status status = stub_->LogIn(&context, request, &empty);
  if (status.ok()) {
    std::cout << "User logged in\n";
    return;
  }
  std::cout << "LogIn error, code: " << status.error_code()
            << ", message: " << status.error_message() << '\n';
}

void NetService::Visit(const ChatMessage& message) {
  messenger::Message msg;
  msg.set_sender("Me");
  msg.set_receiver(":(");
  msg.set_data(message.data);
  stream_->Write(msg);
}

void NetService::Visit([[maybe_unused]] const QuitMessage& message) {
  std::cout << "Net service is shutting down\n";
  is_active_ = false;
}

void NetService::SetOutputBus(NetService::Bus* out_bus) { out_bus_ = out_bus; }

NetService::Bus& NetService::GetBus() { return bus_; }

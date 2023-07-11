#include "net_service.h"

#include <iostream>
#include <thread>

NetService::NetService(const std::shared_ptr<grpc::Channel>& channel)
    : stub_(messenger::Messenger::NewStub(channel)) {}

void NetService::Run() {
  is_active_ = true;
  grpc::ClientContext context;
  stream_ = stub_->OpenMessageStream(&context);

  auto read_thread = std::jthread(&NetService::ReadMessages, this);
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

void NetService::ReadMessages() {
  messenger::Message msg;
  while (stream_->Read(&msg)) {
    auto login = GetUserLogin(msg.sender());
    if (!login.has_value()) {
      std::cout << "[Warning] Cannot get login\n";
      login = std::to_string(msg.sender());
    }
    std::cout << "New message!\nSender: " << login.value()
              << "\nChat id: " << msg.chat_id() << "\nData: " << msg.data()
              << '\n';
    if (!chats_.contains(msg.chat_id())) {
      auto chat_sum = GetChatSummary(msg.chat_id());
      if (!chat_sum.has_value()) {
        std::cout << "Cannot get chat sum\n";
        continue;
      }
      std::scoped_lock lk(m_);
      chats_.emplace(msg.chat_id(), chat_sum.value());
    }
  }
}

void NetService::LogOut() {
  if (!logged_in_) {
    return;
  }
  messenger::LogOutRequest request;
  request.set_id(id_);
  google::protobuf::Empty empty;

  grpc::ClientContext context;
  auto status = stub_->LogOut(&context, request, &empty);
  std::cout << "User logged out\n";
}

void NetService::Visit(const LoginMessage& message) {
  if (logged_in_) {
    std::cout << "Already logged in\n";
    return;
  }
  messenger::LogInRequest request;
  request.set_login(message.login);
  request.set_password(message.password);
  messenger::LogInResponce response;

  grpc::ClientContext login_context;
  if (message.signup) {
    auto status = stub_->SignUp(&login_context, request, &response);
    if (!status.ok()) {
      std::cout << "Cannot sign up: " << status.error_code() << " | "
                << status.error_message() << '\n';
      return;
    }
    std::cout << "Signed up\n";
  } else {
    grpc::Status status = stub_->LogIn(&login_context, request, &response);
    if (!status.ok()) {
      std::cout << "LogIn error, code: " << status.error_code()
                << ", message: " << status.error_message() << '\n';
      return;
    }
  }
  id_ = response.id();

  messenger::Message init_msg;
  init_msg.set_sender(id_);
  stream_->Write(init_msg);

  SetupInfo();
}

void NetService::Visit(const ChatMessage& message) {
  if (!logged_in_) {
    std::cout << "Log in first\n";
  }
  std::shared_lock lk(m_);
  if (!chats_.contains(message.chat_id)) {
    std::cout << "I don't know this chat\n";
    return;
  }
  lk.unlock();
  messenger::Message msg;
  msg.set_sender(id_);
  msg.set_chat_id(message.chat_id);
  msg.set_data(message.data);
  stream_->Write(msg);
}

void NetService::Visit([[maybe_unused]] const QuitMessage& message) {
  std::cout << "Net service is shutting down\n";
  is_active_ = false;
}

void NetService::Visit([[maybe_unused]] const PrintChatsInfoMessage& message) {
  if (!logged_in_) {
    std::cout << "Log in first\n";
  }
  std::shared_lock lk(m_);
  if (chats_.empty()) {
    std::cout << "No chats.\n";
    return;
  }
  auto func = [this](const auto& p) { PrintChatInfo(p); };
  std::for_each(chats_.cbegin(), chats_.cend(), func);
}

void NetService::Visit(const CreateChatMessage& message) {
  if (!logged_in_) {
    std::cout << "Log in first\n";
  }
  messenger::CreateChatRequest request;
  std::unordered_set<Id> ids;
  ids.reserve(message.logins.size());
  std::for_each(message.logins.cbegin(), message.logins.cend(),
                [this, &ids](const auto& s) {
                  auto id = GetUserId(s);
                  if (id.has_value()) {
                    ids.emplace(id.value());
                  }
                });
  if (ids.empty()) {
    std::cout << "Empty list of valid users\n";
    return;
  }
  ids.emplace(id_);
  request.mutable_users()->Assign(ids.begin(), ids.end());
  messenger::CreateChatResponse responce;
  grpc::ClientContext context;
  auto status = stub_->CreateChat(&context, request, &responce);
  auto chat_id = responce.chat_id();
  auto chat_sum = GetChatSummary(chat_id);
  if (!chat_sum.has_value()) {
    std::cout << "Cannot create chat.\n";
    return;
  }
  std::cout << "Chat created, id: " << chat_id << '\n';
  std::unique_lock lk(m_);
  chats_.emplace(chat_id, std::move(chat_sum.value()));
}

void NetService::PrintChatInfo(const std::pair<Id, messenger::ChatSummary>& p) {
  const auto& sum = p.second;
  std::cout << "Chat " << sum.id() << "\nUsers: ";
  for (auto it = sum.users().cbegin(); it != sum.users().cend(); ++it) {
    auto login = GetUserLogin(*it);
    if (!login.has_value()) {
      continue;
    }
    if (it != sum.users().cbegin()) {
      std::cout << ", ";
    }
    std::cout << login.value();
  }
  //  std::for_each(sum.users().cbegin(), sum.users().cend(), [this](Id id) {
  //    auto login = GetUserLogin(id);
  //    if (login.has_value()) {
  //      std::cout << login.value() << ", ";
  //    }
  //  });
  std::cout << '\n';
}

void NetService::SetOutputBus(NetService::Bus* out_bus) { out_bus_ = out_bus; }

NetService::Bus& NetService::GetBus() { return bus_; }

void NetService::SetupInfo() {
  google::protobuf::UInt64Value id_msg;
  id_msg.set_value(id_);
  messenger::UserSummary sum;
  grpc::ClientContext sum_context;
  stub_->GetUserSummary(&sum_context, id_msg, &sum);

  messenger::GetChatsRequest chats_request;
  chats_request.mutable_chat_ids()->CopyFrom(sum.chat_ids());
  messenger::GetChatsResponse chats_responce;
  grpc::ClientContext chats_context;
  stub_->GetChats(&chats_context, chats_request, &chats_responce);
  std::for_each(
      chats_responce.chats().cbegin(), chats_responce.chats().cend(),
      [&c = chats_](const auto& c_sum) { c.emplace(c_sum.id(), c_sum); });
  logged_in_ = true;
  std::cout << "User logged in\n";
}

std::optional<messenger::ChatSummary> NetService::GetChatSummary(Id chat_id) {
  messenger::GetChatsRequest chats_request;
  chats_request.add_chat_ids(chat_id);
  messenger::GetChatsResponse chats_responce;
  grpc::ClientContext chats_context;
  stub_->GetChats(&chats_context, chats_request, &chats_responce);
  return chats_responce.chats().at(0);
}

std::optional<NetService::Id> NetService::GetUserId(const std::string& login) {
  google::protobuf::StringValue login_msg;
  login_msg.set_value(login);
  google::protobuf::UInt64Value id_msg;
  grpc::ClientContext context;
  auto status = stub_->GetId(&context, login_msg, &id_msg);
  if (!status.ok()) {
    std::cout << "Cannot get id. Error code " << status.error_code() << ", "
              << status.error_message() << '\n';
    return std::nullopt;
  }
  return id_msg.value();
}

std::optional<std::string> NetService::GetUserLogin(Id id) {
  google::protobuf::StringValue login_msg;
  google::protobuf::UInt64Value id_msg;
  id_msg.set_value(id);
  grpc::ClientContext context;
  auto status = stub_->GetLogin(&context, id_msg, &login_msg);
  if (!status.ok()) {
    std::cout << "Cannot get login. Error code " << status.error_code() << ", "
              << status.error_message() << '\n';
    return std::nullopt;
  }
  return login_msg.value();
}

#include "server_implementation.h"

MessengerServiceImpl::MessengerServiceImpl() {
  auto credentials = credentials_db_.LoadData();
  if (credentials.status == CredentialsDb::CredentialsResponce::kOk) {
    std::for_each(credentials.data->cbegin(), credentials.data->cend(),
                  [&d_db = details_db_](const auto& pair) {
                    d_db.AddId(pair.second.login(), pair.second.id());
                  });
  }
  chats_db_.LoadData();
  users_db_.LoadData();
}

grpc::Status MessengerServiceImpl::SignUp(
    [[maybe_unused]] ServerContext* context,
    const messenger::LogInRequest* request,
    messenger::LogInResponce* response) {
  std::cout << "SignUp request\nLogin: " << request->login()
            << "\nPassword: " << request->password() << "\n";

  auto db_response =
      credentials_db_.AddUser(request->login(), request->password());
  if (db_response.status != CredentialsDb::Responce::kOk) {
    std::cout << "Cannot add user. Error code: " << db_response.status << "\n";
    return Status{grpc::UNAUTHENTICATED, std::to_string(db_response.status)};
  }

  details_db_.AddId(request->login(), db_response.id);
  details_db_.AddActive(db_response.id);
  std::cout << "User added " << db_response.id << "\n";
  response->set_id(db_response.id);
  return Status::OK;
}

grpc::Status MessengerServiceImpl::LogIn(
    [[maybe_unused]] ServerContext* context,
    const messenger::LogInRequest* request,
    messenger::LogInResponce* response) {
  std::cout << "LogIn request\nLogin: " << request->login()
            << "\nPassword: " << request->password() << "\n";

  auto db_response =
      credentials_db_.CheckCredentials(request->login(), request->password());
  if (db_response.status != CredentialsDb::Responce::kOk) {
    std::cout << "Invalid credentials. Error code: " << db_response.status
              << "\n";
    return Status{grpc::UNAUTHENTICATED, std::to_string(db_response.status)};
  }

  std::cout << "Valid credentials\n";
  details_db_.AddActive(db_response.id);
  response->set_id(db_response.id);
  return Status::OK;
}

grpc::Status MessengerServiceImpl::LogOut(
    [[maybe_unused]] ServerContext* context,
    const messenger::LogOutRequest* request, [[maybe_unused]] Empty* responce) {
  std::cout << "LogOut request:\nId: " << request->id() << '\n';
  details_db_.RemoveActive(request->id());
  return Status::OK;
}

grpc::Status MessengerServiceImpl::OpenMessageStream(
    [[maybe_unused]] ServerContext* context,
    grpc::ServerReaderWriter<Message, Message>* stream) {
  Message msg;
  auto flag = stream->Read(&msg);
  if (!flag) {
    return Status::OK;
  }
  uint64_t id = msg.sender();
  message_bus_.Insert(id, Queue<Message>());
  std::cout << "Message stream opened to user " << id << ".\n";

  auto write_thread = std::jthread(
      [stream, &bus = message_bus_, id](const std::stop_token& token) {
        auto func = [stream, &token](auto& queue) {
          while (!token.stop_requested()) {
            std::optional<Message> msg = queue.TryPop();
            if (!msg.has_value()) {
              std::this_thread::yield();
            } else {
              stream->Write(msg.value());
            }
          }
        };
        bus.ApplySoft(id, std::move(func));
      });

  while (stream->Read(&msg)) {
    auto chat_id = msg.chat_id();
    std::cout << "New message:\nSender: " << msg.sender()
              << "\nChat id: " << chat_id << "\nData: " << msg.data() << '\n';
    chats_db_.AddMessage(chat_id, msg);
    auto chat_sum = chats_db_.GetChatSummary(chat_id);
    if (!chat_sum.has_value()) {
      std::cout << "Invalid chat!" << '\n';
      continue;
    }
    const auto& k_udata = chat_sum.value().users();
    std::for_each(k_udata.cbegin(), k_udata.cend(),
                  [&msg, &d_db = details_db_, &bus = message_bus_,
                   sender = msg.sender()](uint64_t id) {
                    if (d_db.IsActive(id) && id != sender) {
                      bus.ApplySoft(id,
                                    [&msg](auto& queue) { queue.Push(msg); });
                    }
                  });
  }
  write_thread.request_stop();

  message_bus_.Erase(id);
  std::cout << "Message stream closed.\n";
  return Status::OK;
}

grpc::Status MessengerServiceImpl::GetUserSummary(
    [[maybe_unused]] ServerContext* context,
    const google::protobuf::UInt64Value* user_id,
    messenger::UserSummary* login) {
  auto summary_opt = users_db_.GetSummary(user_id->value());
  if (!summary_opt.has_value()) {
    return Status{grpc::NOT_FOUND, "No user with this id."};
  }
  *login = summary_opt.value();

  return Status::OK;
}

grpc::Status MessengerServiceImpl::GetChats(
    [[maybe_unused]] ServerContext* context,
    const messenger::GetChatsRequest* request,
    messenger::GetChatsResponse* response) {
  messenger::GetChatsResponse chats;
  std::for_each(request->chat_ids().cbegin(), request->chat_ids().cend(),
                [&chats, &c_db = chats_db_](uint64_t id) {
                  auto* new_summary = chats.add_chats();
                  auto sum_opt = c_db.GetChatSummary(id);
                  if (!sum_opt.has_value()) {
                    std::cout << "No such chat " << id << "\n";
                    return;
                  }
                  *new_summary = sum_opt.value();
                });
  *response = chats;
  return Status::OK;
}

grpc::Status MessengerServiceImpl::GetId(
    [[maybe_unused]] ServerContext* context,
    const google::protobuf::StringValue* login,
    google::protobuf::UInt64Value* id) {
  auto id_opt = details_db_.GetId(login->value());
  if (!id_opt.has_value()) {
    return Status{grpc::NOT_FOUND, "Invalid login."};
  }
  id->set_value(id_opt.value());
  return Status::OK;
}

grpc::Status MessengerServiceImpl::GetLogin(
    [[maybe_unused]] ServerContext* context,
    const google::protobuf::UInt64Value* id,
    google::protobuf::StringValue* login) {
  auto id_opt = details_db_.GetLogin(id->value());
  if (!id_opt.has_value()) {
    return Status{grpc::NOT_FOUND, "Invalid id."};
  }
  login->set_value(id_opt.value());
  return Status::OK;
}

grpc::Status MessengerServiceImpl::CreateChat(
    [[maybe_unused]] ServerContext* context,
    const messenger::CreateChatRequest* request,
    messenger::CreateChatResponse* responce) {
  std::unordered_set<uint64_t> ids(request->users().cbegin(),
                                   request->users().cend());
  uint64_t id = chats_db_.CreateChat(ids);
  std::for_each(
      ids.cbegin(), ids.cend(),
      [c_id = id, &u_db = users_db_](uint64_t id) { u_db.AddChat(id, c_id); });
  responce->set_chat_id(id);
  return Status::OK;
}

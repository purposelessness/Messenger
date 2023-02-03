#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "../threadsafe/map.h"
#include "../threadsafe/queue.h"
#include "database/chats_db.h"
#include "database/credentials_db.h"
#include "database/details_db.h"
#include "database/users_db.h"
#include "messenger.grpc.pb.h"

using google::protobuf::Empty;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using messenger::Message;
using messenger::Messenger;

class MessengerServiceImpl final : public Messenger::Service {
 public:
  MessengerServiceImpl() {
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

  Status SignUp([[maybe_unused]] ServerContext* context,
                const messenger::LogInRequest* request,
                messenger::LogInResponce* response) override {
    std::cout << "SignUp request\nLogin: " << request->login()
              << "\nPassword: " << request->password() << "\n";

    auto db_response =
        credentials_db_.AddUser(request->login(), request->password());
    if (db_response.status != CredentialsDb::Responce::kOk) {
      std::cout << "Cannot add user. Error code: " << db_response.status
                << "\n";
      return Status{grpc::UNAUTHENTICATED, std::to_string(db_response.status)};
    }

    std::cout << "User added\n";
    response->set_id(db_response.id);
    return Status::OK;
  }

  Status LogIn([[maybe_unused]] ServerContext* context,
               const messenger::LogInRequest* request,
               messenger::LogInResponce* response) override {
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

  Status LogOut([[maybe_unused]] ServerContext* context,
                const messenger::LogOutRequest* request,
                [[maybe_unused]] Empty* responce) override {
    std::cout << "LogOut request:\nId: " << request->id() << '\n';
    details_db_.RemoveActive(request->id());
    return Status::OK;
  }

  Status OpenMessageStream(
      [[maybe_unused]] ServerContext* context,
      grpc::ServerReaderWriter<Message, Message>* stream) override {
    std::cout << "Message stream opened.\n";
    std::atomic<bool> open = true;

    Message msg;
    stream->Read(&msg);
    uint64_t id = msg.sender();
    message_bus_.Insert(id, Queue<Message>());

    auto write_thread =
        std::jthread([stream, &bus = message_bus_, &open, id]() {
          auto func = [stream, &open](auto& queue) {
            while (open) {
              std::optional<Message> msg = queue.TryPop();
              if (!msg.has_value()) {
                std::this_thread::yield();
              }
              stream->Write(msg.value());
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
      const auto& kUdata = chat_sum.value().users();
      std::for_each(
          kUdata.cbegin(), kUdata.cend(),
          [&msg, &d_db = details_db_, &bus = message_bus_](uint64_t id) {
            if (d_db.IsActive(id)) {
              bus.ApplySoft(id, [&msg](auto& queue) { queue.Push(msg); });
            }
          });
    }

    open = false;
    message_bus_.Erase(id);
    std::cout << "Message stream closed.\n";
    return Status::OK;
  }

  Status GetUserSummary([[maybe_unused]] ServerContext* context,
                        const google::protobuf::UInt64Value* user_id,
                        messenger::UserSummary* login) override {
    auto summary_opt = users_db_.GetSummary(user_id->value());
    if (!summary_opt.has_value()) {
      return Status{grpc::NOT_FOUND, "No user with this id."};
    }
    *login = summary_opt.value();

    return Status::OK;
  }

  Status GetChats([[maybe_unused]] ServerContext* context,
                  const messenger::UserChatsRequest* request,
                  messenger::UserChats* response) override {
    messenger::UserChats chats;
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

  Status GetId([[maybe_unused]] ServerContext* context,
               const google::protobuf::StringValue* login,
               google::protobuf::UInt64Value* id) override {
    auto id_opt = details_db_.GetId(login->value());
    if (!id_opt.has_value()) {
      return Status{grpc::NOT_FOUND, "Invalid login."};
    }
    id->set_value(id_opt.value());
    return Status::OK;
  }

  Status GetLogin([[maybe_unused]] ServerContext* context,
                  const google::protobuf::UInt64Value* id,
                  google::protobuf::StringValue* login) override {
    auto id_opt = details_db_.GetLogin(id->value());
    if (!id_opt.has_value()) {
      return Status{grpc::NOT_FOUND, "Invalid id."};
    }
    login->set_value(id_opt.value());
    return Status::OK;
  }

  Status CreateChat([[maybe_unused]] ServerContext* context,
                    const messenger::CreateChatRequest* request,
                    messenger::CreateChatResponse* responce) override {
    std::unordered_set<uint64_t> ids(request->users().cbegin(),
                                     request->users().cend());
    uint64_t id = chats_db_.CreateChat(ids);
    std::for_each(ids.cbegin(), ids.cend(),
                  [c_id = id, &u_db = users_db_](uint64_t id) {
                    u_db.AddChat(id, c_id);
                  });
    responce->set_chat_id(id);
    return Status::OK;
  }

 private:
  CredentialsDb credentials_db_;
  DetailsDb details_db_;
  UsersDb users_db_;
  ChatsDb chats_db_;

  Map<uint64_t, Queue<Message>> message_bus_;
};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  MessengerServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << '\n';

  server->Wait();
}

int main() {
  RunServer();
  return 0;
}

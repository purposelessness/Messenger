#ifndef MESSENGER_SERVER_SERVER_IMPLEMENTATION_H_
#define MESSENGER_SERVER_SERVER_IMPLEMENTATION_H_

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

class MessengerServiceImpl final : public messenger::Messenger::Service {
  using Empty = google::protobuf::Empty;
  using Server = grpc::Server;
  using ServerBuilder = grpc::ServerBuilder;
  using ServerContext = grpc::ServerContext;
  using Status = grpc::Status;
  using Message = messenger::Message;

 public:
  MessengerServiceImpl();

  Status SignUp([[maybe_unused]] ServerContext* context,
                const messenger::LogInRequest* request,
                messenger::LogInResponce* response) override;

  Status LogIn([[maybe_unused]] ServerContext* context,
               const messenger::LogInRequest* request,
               messenger::LogInResponce* response) override;

  Status LogOut([[maybe_unused]] ServerContext* context,
                const messenger::LogOutRequest* request,
                [[maybe_unused]] Empty* responce) override;

  Status OpenMessageStream(
      [[maybe_unused]] ServerContext* context,
      grpc::ServerReaderWriter<Message, Message>* stream) override;

  Status GetUserSummary([[maybe_unused]] ServerContext* context,
                        const google::protobuf::UInt64Value* user_id,
                        messenger::UserSummary* login) override;

  Status GetChats([[maybe_unused]] ServerContext* context,
                  const messenger::GetChatsRequest* request,
                  messenger::GetChatsResponse* response) override;

  Status GetId([[maybe_unused]] ServerContext* context,
               const google::protobuf::StringValue* login,
               google::protobuf::UInt64Value* id) override;

  Status GetLogin([[maybe_unused]] ServerContext* context,
                  const google::protobuf::UInt64Value* id,
                  google::protobuf::StringValue* login) override;

  Status CreateChat([[maybe_unused]] ServerContext* context,
                    const messenger::CreateChatRequest* request,
                    messenger::CreateChatResponse* responce) override;

 private:
  CredentialsDb credentials_db_;
  DetailsDb details_db_;
  UsersDb users_db_;
  ChatsDb chats_db_;

  Map<uint64_t, Queue<Message>> message_bus_;
};

#endif  // MESSENGER_SERVER_SERVER_IMPLEMENTATION_H_

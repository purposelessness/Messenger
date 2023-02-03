#ifndef MESSENGER_NET_NET_SERVICE_H_
#define MESSENGER_NET_NET_SERVICE_H_

#include <grpcpp/grpcpp.h>

#include <shared_mutex>

#include "../message.h"
#include "../message_visitor.h"
#include "../threadsafe/queue.h"
#include "messenger.grpc.pb.h"

class NetService : public IMessageVisitor {
  using Bus = Queue<std::unique_ptr<Message>>;
  using Id = uint64_t;

 public:
  explicit NetService(const std::shared_ptr<grpc::Channel>& channel);

  void Run();

  void Visit(const LoginMessage& message) override;
  void Visit(const ChatMessage& message) override;
  void Visit(const QuitMessage& message) override;
  void Visit(const PrintChatsInfoMessage& message) override;
  void Visit(const CreateChatMessage& message) override;

  void SetOutputBus(Bus* out_bus);
  Bus& GetBus();

 private:
  void SetupInfo();
  void LogOut();
  void ReadMessages();
  void PrintChatInfo(const std::pair<Id, messenger::ChatSummary>& p);
  std::optional<messenger::ChatSummary> GetChatSummary(Id chat_id);
  std::optional<Id> GetUserId(const std::string& login);
  std::optional<std::string> GetUserLogin(Id id);

  std::unique_ptr<messenger::Messenger::Stub> stub_;
  std::shared_ptr<
      grpc::ClientReaderWriter<messenger::Message, messenger::Message>>
      stream_;
  grpc::ClientContext writer_context_;

  Bus* out_bus_ = nullptr;
  Bus bus_;
  bool is_active_ = false;
  bool logged_in_ = false;

  Id id_ = 0;
  std::unordered_map<Id, messenger::ChatSummary> chats_;
  std::shared_mutex m_;
};

#endif  // MESSENGER_NET_NET_SERVICE_H_

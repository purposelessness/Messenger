#ifndef MESSENGER_NET_NET_SERVICE_H_
#define MESSENGER_NET_NET_SERVICE_H_

#include <grpcpp/grpcpp.h>

#include "../message.h"
#include "../message_visitor.h"
#include "../queue.h"
#include "messenger.grpc.pb.h"

class NetService : public MessageVisitor {
  using Bus = Queue<std::unique_ptr<Message>>;

 public:
  NetService(std::shared_ptr<grpc::Channel> channel);

  void Run();

  void Visit(const LoginMessage& message) override;
  void Visit(const ChatMessage& message) override;
  void Visit(const QuitMessage& message) override;

  void SetOutputBus(Bus* out_bus);
  Bus& GetBus();

 private:
  void LogOut();

  std::unique_ptr<messenger::Messenger::Stub> stub_;
  std::shared_ptr<
      grpc::ClientReaderWriter<messenger::Message, messenger::Message>>
      stream_;
  grpc::ClientContext writer_context_;

  Bus* out_bus_ = nullptr;
  Bus bus_;
  bool is_active_ = false;
};

#endif  // MESSENGER_NET_NET_SERVICE_H_

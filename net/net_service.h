#ifndef MESSENGER_NET_NET_SERVICE_H_
#define MESSENGER_NET_NET_SERVICE_H_

#include "../message.h"
#include "../message_visitor.h"
#include "../queue.h"

class NetService : public MessageVisitor {
  using Bus = Queue<std::unique_ptr<Message>>;

 public:
  void Run();

  void Visit(const LoginMessage& message) override;
  void Visit(const ChatMessage& message) override;
  void Visit(const QuitMessage& message) override;

  void SetOutputBus(Bus* out_bus);
  Bus& GetBus();

 private:
  Bus* out_bus_ = nullptr;
  Bus bus_;
  bool is_active_ = false;
};

#endif  // MESSENGER_NET_NET_SERVICE_H_
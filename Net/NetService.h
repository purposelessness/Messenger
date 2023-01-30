#ifndef MESSENGER__NETSERVICE_H_
#define MESSENGER__NETSERVICE_H_

#include "../Message.h"
#include "../MessageVisitor.h"
#include "../Queue.h"

class NetService : public MessageVisitor {
  using Bus = Queue<std::unique_ptr<Message>>;

 public:
  void run();

  void visit(const LoginMessage& message) override;
  void visit(const ChatMessage& message) override;
  void visit(const QuitMessage& message) override;

  void setOutputBus(Bus* out_bus);
  Bus& getBus();

 private:
  Bus* _out_bus = nullptr;
  Bus _bus;
  bool _is_active = false;
};

#endif  // MESSENGER__NETSERVICE_H_
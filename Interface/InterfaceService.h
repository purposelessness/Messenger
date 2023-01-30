#ifndef MESSENGER__INTERFACESERVICE_H_
#define MESSENGER__INTERFACESERVICE_H_

#include <functional>
#include <unordered_map>

#include "../Message.h"
#include "../MessageVisitor.h"
#include "../Queue.h"

class InterfaceService : public MessageVisitor {
  using Bus = Queue<std::unique_ptr<Message>>;

 public:
  InterfaceService();

  void run();
  static void printInfo();
  void process();

  void quit();
  void login();
  void enterMessage();

  void visit(const LoginMessage& message) override;
  void visit(const ChatMessage& message) override;
  void visit(const QuitMessage& message) override;

  void setOutputBus(Bus* out_bus);
  Bus& getBus();

  [[nodiscard]] bool isActive() const;

 private:
  Bus* _out_bus = nullptr;
  Bus _bus;
  bool _is_active = false;

  std::unordered_map<char, std::function<void()>> _command_map = {};
};

#endif  // MESSENGER__INTERFACESERVICE_H_
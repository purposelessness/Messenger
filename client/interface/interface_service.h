#ifndef MESSENGER_INTERFACE_INTERFACE_SERVICE_H_
#define MESSENGER_INTERFACE_INTERFACE_SERVICE_H_

#include <functional>
#include <unordered_map>

#include "../message.h"
#include "../message_visitor.h"
#include "../threadsafe/queue.h"

class InterfaceService : public IMessageVisitor {
  using Bus = Queue<std::unique_ptr<Message>>;

 public:
  InterfaceService();

  void Run();
  static void PrintInfo();
  void Process();

  void Quit();
  void Signup();
  void Login();
  void PrintChatsInfo();
  void EnterMessage();
  void CreateChat();

  void Visit(const LoginMessage& message) override;
  void Visit(const ChatMessage& message) override;
  void Visit(const QuitMessage& message) override;
  void Visit(const PrintChatsInfoMessage &message) override;
  void Visit(const CreateChatMessage &message) override;

  void SetOutputBus(Bus* out_bus);
  Bus& GetBus();

  [[nodiscard]] bool IsActive() const;

 private:
  Bus* out_bus_ = nullptr;
  Bus bus_;
  bool is_active_ = false;

  std::unordered_map<char, std::function<void()>> command_map_ = {};
};

#endif  // MESSENGER_INTERFACE_INTERFACE_SERVICE_H_

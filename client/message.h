#ifndef MESSENGER__MESSAGE_H_
#define MESSENGER__MESSAGE_H_

#include <string>
#include <utility>
#include <vector>

class IMessageVisitor;

struct Message {
  virtual ~Message() = default;
  virtual void Accept(IMessageVisitor& visitor) = 0;
};

struct LoginMessage : Message {
  LoginMessage(std::string login, std::string password, bool signup = false);

  void Accept(IMessageVisitor& visitor) override;
  std::string login;
  std::string password;
  bool signup;
};

struct ChatMessage : Message {
  ChatMessage(uint64_t chat_id, std::string data);

  void Accept(IMessageVisitor& visitor) override;
  uint64_t chat_id;
  std::string data;
};

struct QuitMessage : Message {
  void Accept(IMessageVisitor& visitor) override;
};

struct PrintChatsInfoMessage : Message {
  void Accept(IMessageVisitor& visitor) override;
};

struct CreateChatMessage : Message {
  explicit CreateChatMessage(std::vector<std::string> logins);

  void Accept(IMessageVisitor& visitor) override;
  std::vector<std::string> logins;
};

#endif  // MESSENGER__MESSAGE_H_

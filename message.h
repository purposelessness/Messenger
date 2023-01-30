#ifndef MESSENGER__MESSAGE_H_
#define MESSENGER__MESSAGE_H_

#include <string>
#include <utility>

class MessageVisitor;

struct Message {
  virtual ~Message() = default;
  virtual void Accept(MessageVisitor& visitor) = 0;
};

struct LoginMessage : Message {
  LoginMessage(std::string login, std::string password);

  void Accept(MessageVisitor& visitor) override;
  std::string login;
  std::string password;
};

struct ChatMessage : Message {
  explicit ChatMessage(std::string data);

  void Accept(MessageVisitor& visitor) override;
  std::string data;
};

struct QuitMessage : Message {
  void Accept(MessageVisitor& visitor) override;
};

#endif  // MESSENGER__MESSAGE_H_

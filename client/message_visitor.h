#ifndef MESSENGER__MESSAGE_VISITOR_H_
#define MESSENGER__MESSAGE_VISITOR_H_

#include "message.h"

class IMessageVisitor {
 public:
  virtual ~IMessageVisitor() = default;
  virtual void Visit(const LoginMessage& message) = 0;
  virtual void Visit(const ChatMessage& message) = 0;
  virtual void Visit(const QuitMessage& message) = 0;
  virtual void Visit(const PrintChatsInfoMessage& message) = 0;
  virtual void Visit(const CreateChatMessage& message) = 0;
};

#endif  // MESSENGER__MESSAGE_VISITOR_H_

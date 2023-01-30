#ifndef MESSENGER__MESSAGEVISITOR_H_
#define MESSENGER__MESSAGEVISITOR_H_

#include "Message.h"

class MessageVisitor {
 public:
  virtual ~MessageVisitor() = default;
  virtual void visit(const LoginMessage& message) = 0;
  virtual void visit(const ChatMessage& message) = 0;
  virtual void visit(const QuitMessage& message) = 0;
};

#endif  // MESSENGER__MESSAGEVISITOR_H_
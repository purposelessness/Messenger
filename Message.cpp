#include "Message.h"

#include <utility>

#include "MessageVisitor.h"

void LoginMessage::accept(MessageVisitor& visitor) { visitor.visit(*this); }

LoginMessage::LoginMessage(std::string login, std::string password)
    : login(std::move(login)), password(std::move(password)) {}

void ChatMessage::accept(MessageVisitor& visitor) { visitor.visit(*this); }

ChatMessage::ChatMessage(std::string data) : data(std::move(data)) {}

void QuitMessage::accept(MessageVisitor& visitor) { visitor.visit(*this); }

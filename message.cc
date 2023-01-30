#include "message.h"

#include <utility>

#include "message_visitor.h"

void LoginMessage::Accept(MessageVisitor& visitor) { visitor.Visit(*this); }

LoginMessage::LoginMessage(std::string login, std::string password)
    : login(std::move(login)), password(std::move(password)) {}

void ChatMessage::Accept(MessageVisitor& visitor) { visitor.Visit(*this); }

ChatMessage::ChatMessage(std::string data) : data(std::move(data)) {}

void QuitMessage::Accept(MessageVisitor& visitor) { visitor.Visit(*this); }

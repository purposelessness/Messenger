#include "message.h"

#include <utility>

#include "message_visitor.h"

void LoginMessage::Accept(IMessageVisitor& visitor) { visitor.Visit(*this); }

LoginMessage::LoginMessage(std::string login, std::string password, bool signup)
    : login(std::move(login)), password(std::move(password)), signup(signup) {}

void ChatMessage::Accept(IMessageVisitor& visitor) { visitor.Visit(*this); }

ChatMessage::ChatMessage(uint64_t chat_id, std::string data)
    : chat_id(chat_id), data(std::move(data)) {}

void QuitMessage::Accept(IMessageVisitor& visitor) { visitor.Visit(*this); }

void PrintChatsInfoMessage::Accept(IMessageVisitor& visitor) {
  visitor.Visit(*this);
}

CreateChatMessage::CreateChatMessage(std::vector<std::string> logins)
    : logins(std::move(logins)) {}

void CreateChatMessage::Accept(IMessageVisitor& visitor) {
  visitor.Visit(*this);
}

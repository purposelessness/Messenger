#include "users_db.h"

#include <mutex>

std::optional<UsersDb::Id> UsersDb::GetChat(Id user_id, Id companion_id) {
  std::shared_lock lk(m_);
  if (!chat_data_.contains(user_id) ||
      !chat_data_[user_id].contains(companion_id)) {
    return std::nullopt;
  }
  std::optional out{chat_data_[user_id][companion_id]};
  return out;
}

void UsersDb::AddChat(Id user_id, Id companion_id, Id chat_id) {
  std::scoped_lock lk(m_);
  chat_data_[user_id][companion_id] = chat_id;
}

void UsersDb::RemoveUser(Id user_id) {
  std::scoped_lock lk(m_);
  chat_data_.erase(user_id);
}

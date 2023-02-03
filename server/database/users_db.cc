#include "users_db.h"

#include <algorithm>
#include <fstream>
#include <mutex>

UsersDb::UsersDb(std::string filename) : filename_(std::move(filename)) {}

std::optional<messenger::UserSummary> UsersDb::GetSummary(Id user_id) {
  std::shared_lock lk(m_);
  if (!user_data_.contains(user_id)) {
    return std::nullopt;
  }
  return user_data_[user_id];
}

void UsersDb::AddChat(Id user_id, Id chat_id) {
  std::scoped_lock lk(m_);
  user_data_[user_id].add_chat_ids(chat_id);
}

void UsersDb::RemoveUser(Id user_id) {
  std::scoped_lock lk(m_);
  user_data_.erase(user_id);
}

void UsersDb::LoadData() {
  std::ifstream file(filename_, std::ios::binary);
  if (!file.good()) {
    return;
  }
  messenger::UserSummaryBook chat_book;
  if (!chat_book.ParseFromIstream(&file)) {
    return;
  }

  auto data = chat_book.data();
  std::unordered_map<Id, messenger::UserSummary> out;
  out.reserve(data.size());
  std::for_each(std::make_move_iterator(data.begin()),
                std::make_move_iterator(data.end()),
                [&out](messenger::UserSummary&& u) {
                  out.emplace(u.id(), std::move(u));
                });

  google::protobuf::ShutdownProtobufLibrary();

  user_data_ = std::move(out);
}

void UsersDb::SaveData() {
  std::ofstream file(filename_, std::ios::trunc | std::ios::binary);
  if (!file.good()) {
    return;
  }
  messenger::UserSummaryBook user_book;
  std::for_each(user_data_.begin(), user_data_.end(), [&user_book](const auto& pair) {
    auto* entry = user_book.add_data();
    *entry = pair.second;
  });

  if (!user_book.SerializeToOstream(&file)) {
    return;
  }

  google::protobuf::ShutdownProtobufLibrary();
}


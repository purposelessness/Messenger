#include "chats_db.h"

#include <algorithm>
#include <fstream>

ChatsDb::ChatsDb(std::string filename) : filename_(std::move(filename)) {}

std::optional<messenger::Chat> ChatsDb::GetChatHistory(Id chat_id,
                                                       uint64_t depth) {
  std::shared_lock s_lk(m_);
  if (!data_.contains(chat_id)) {
    return std::nullopt;
  }
  if (depth == 0) {
    return std::optional{data_[chat_id].chat};
  }
  messenger::Chat out;
  std::shared_lock lk(data_[chat_id].m);
  auto& chat = data_[chat_id].chat;
  s_lk.unlock();
  std::for_each_n(chat.data().begin(), depth,
                  [&out](const messenger::Message& msg) {
                    auto* new_msg = out.add_data();
                    *new_msg = msg;
                  });
  return out;
}

std::optional<messenger::Message> ChatsDb::GetLastMessage(Id chat_id) {
  std::shared_lock s_lk(m_);
  if (!data_.contains(chat_id)) {
    return std::nullopt;
  }
  std::shared_lock lk(data_[chat_id].m);
  auto message = *data_[chat_id].chat.data().begin();
  return message;
}

std::optional<messenger::ChatSummary> ChatsDb::GetChatSummary(Id chat_id) {
  std::shared_lock s_lk(m_);
  if (!data_.contains(chat_id)) {
    return std::nullopt;
  }
  std::shared_lock lk(data_[chat_id].m);
  return data_[chat_id].chat.summary();
}

ChatsDb::Id ChatsDb::CreateChat(const std::unordered_set<Id>& users) {
  messenger::ChatSummary sum;
  auto* users_mut = sum.mutable_users();
  users_mut->Reserve(static_cast<int>(users.size()));
  std::for_each(users.begin(), users.end(),
                [users_mut](Id id) { users_mut->Add(id); });

  std::unique_lock lk(m_);
  Id new_id = data_.size();
  messenger::Chat new_chat;
  sum.set_id(new_id);
  new_chat.mutable_summary()->Swap(&sum);
  data_.emplace(new_id, ChatEntry{std::move(new_chat)});

  return new_id;
}

void ChatsDb::AddMessage(Id chat_id, const messenger::Message& message) {
  std::shared_lock s_lk(m_);
  if (!data_.contains(chat_id)) {
    return;
  }
  auto& entry = data_[chat_id];
  std::scoped_lock lk(entry.m);
  auto& chat = entry.chat;
  s_lk.unlock();

  auto* new_msg = chat.add_data();
  *new_msg = message;
}

void ChatsDb::RemoveChat(Id chat_id) {
  std::scoped_lock lk(m_);
  data_.erase(chat_id);
}

ChatsDb::ChatResponce ChatsDb::LoadData() {
  std::ifstream file(filename_, std::ios::binary);
  if (!file.good()) {
    return {ChatResponce::kFileNotFound, nullptr};
  }
  messenger::ChatBook chat_book;
  if (!chat_book.ParseFromIstream(&file)) {
    return {ChatResponce::kParseError, nullptr};
  }

  auto data = chat_book.data();
  std::unordered_map<Id, ChatsDb::ChatEntry> out;
  out.reserve(data.size());
  std::for_each(std::make_move_iterator(data.begin()),
                std::make_move_iterator(data.end()),
                [&out](messenger::Chat&& c) {
                  out.emplace(c.summary().id(), ChatEntry{std::move(c)});
                });

  google::protobuf::ShutdownProtobufLibrary();

  data_ = std::move(out);
  return {ChatResponce::kOk, &data_};
}

ChatsDb::ChatResponce::Status ChatsDb::SaveData() {
  std::ofstream file(filename_, std::ios::trunc | std::ios::binary);
  if (!file.good()) {
    std::cerr << "Chats db: cannot open file.\n";
    return ChatResponce::kFileNotFound;
  }
  messenger::ChatBook chat_book;
  std::for_each(data_.begin(), data_.end(), [&chat_book](const auto& pair) {
    auto* entry = chat_book.add_data();
    *entry = pair.second.chat;
  });

  if (!chat_book.SerializeToOstream(&file)) {
    std::cerr << "Chats db: cannot serialize data.\n";
    return ChatResponce::kParseError;
  }

  std::cout << "Chats db: data is saved.\n";

  google::protobuf::ShutdownProtobufLibrary();
  return ChatResponce::kOk;
}

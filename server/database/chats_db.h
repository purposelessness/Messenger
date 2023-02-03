#ifndef MESSENGER_SERVER_DATABASE_CHATS_DB_H_
#define MESSENGER_SERVER_DATABASE_CHATS_DB_H_

#include <exception>
#include <optional>
#include <shared_mutex>
#include <unordered_map>

#include "messenger.pb.h"

class ChatsDb {
  using Id = uint64_t;
  struct ChatEntry {
    ChatEntry() = default;
    explicit ChatEntry(messenger::Chat&& chat) : chat(std::move(chat)) {}
    ChatEntry(const ChatEntry& other) : chat(other.chat) {}
    ChatEntry& operator=(const ChatEntry& other) {
      if (this == &other) {
        return *this;
      }
      chat = other.chat;
      return *this;
    }
    ChatEntry(ChatEntry&& other) noexcept : chat(std::move(other.chat)) {}
    ChatEntry& operator=(ChatEntry&& other) noexcept {
      if (this == &other) {
        return *this;
      }
      chat = std::move(other.chat);
      return *this;
    }

    messenger::Chat chat;
    std::shared_mutex m;
  };

 public:
  explicit ChatsDb(std::string filename = "chats.data");

  std::optional<messenger::Chat> GetChatHistory(Id chat_id, uint64_t depth = 0);
  std::optional<messenger::Message> GetLastMessage(Id chat_id);
  std::optional<messenger::ChatSummary> GetChatSummary(Id chat_id);

  void AddMessage(Id chat_id, const messenger::Message& message);
  Id CreateChat(const std::unordered_set<Id>& users);
  void RemoveChat(Id chat_id);

  // Not threadsafe
  struct ChatResponce {
    enum Status { kOk, kFileNotFound, kParseError } status;
    const std::unordered_map<Id, ChatEntry>* data;
  };
  ChatResponce LoadData();
  ChatResponce::Status SaveData();

 private:
  std::string filename_;
  std::unordered_map<Id, ChatEntry> data_;
  std::shared_mutex m_;
};

#endif  // MESSENGER_SERVER_DATABASE_CHATS_DB_H_

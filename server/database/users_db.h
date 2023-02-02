#ifndef MESSENGER_SERVER_DATABASE_USERS_DB_H_
#define MESSENGER_SERVER_DATABASE_USERS_DB_H_

#include <cstdint>
#include <optional>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

class UsersDb {
  using Id = uint64_t;

 public:
  std::optional<Id> GetChat(Id user_id, Id companion_id);
  void AddChat(Id user_id, Id companion_id, Id chat_id);
  void RemoveUser(Id user_id);

 private:
  std::unordered_map<Id, std::unordered_map<Id, Id>> chat_data_;
  mutable std::shared_mutex m_;
};

#endif  // MESSENGER_SERVER_DATABASE_USERS_DB_H_

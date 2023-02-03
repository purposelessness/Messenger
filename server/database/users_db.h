#ifndef MESSENGER_SERVER_DATABASE_USERS_DB_H_
#define MESSENGER_SERVER_DATABASE_USERS_DB_H_

#include <cstdint>
#include <optional>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

#include "messenger.pb.h"

class UsersDb {
  using Id = uint64_t;

 public:
  explicit UsersDb(std::string filename = "users.data");

  std::optional<messenger::UserSummary> GetSummary(Id user_id);
  void AddChat(Id user_id, Id chat_id);
  void RemoveUser(Id user_id);

  void LoadData();
  void SaveData();

 private:
  std::string filename_;
  std::unordered_map<Id, messenger::UserSummary> user_data_;
  mutable std::shared_mutex m_;
};

#endif  // MESSENGER_SERVER_DATABASE_USERS_DB_H_

#ifndef MESSENGER_SERVER_DATABASE_ACTIVE_DB_H_
#define MESSENGER_SERVER_DATABASE_ACTIVE_DB_H_

#include <cstdint>
#include <unordered_set>

#include "../../threadsafe/map.h"
#include "../../threadsafe/set.h"

class DetailsDb {
 public:
  void AddId(const std::string& login, uint64_t id);
  void RemoveId(const std::string& login);
  std::optional<uint64_t> GetId(const std::string& login);

  void AddActive(uint64_t id);
  void RemoveActive(uint64_t id);
  bool IsActive(uint64_t id);

 private:
  static constexpr uint64_t kIdDataSize = 100'003;
  Map<std::string, uint64_t> id_data_{kIdDataSize};
  Set<uint64_t> active_data_;
};

#endif  // MESSENGER_SERVER_DATABASE_ACTIVE_DB_H_

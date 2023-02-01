#ifndef MESSENGER_DATABASE_DATABASE_SERVICE_H_
#define MESSENGER_DATABASE_DATABASE_SERVICE_H_

#include <cstddef>
#include <optional>
#include <string>

class DatabaseService {
 public:
  explicit DatabaseService(std::string credentials_db_name = "credentials.txt");

  std::optional<uint64_t> CheckCredentials(const std::string& login,
                                           const std::string& password);

 private:
  std::string credentials_db_name_;
};

#endif  // MESSENGER_DATABASE_DATABASE_SERVICE_H_

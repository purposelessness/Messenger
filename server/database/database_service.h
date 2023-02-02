#ifndef MESSENGER_SERVER_DATABASE_DATABASE_SERVICE_H_
#define MESSENGER_SERVER_DATABASE_DATABASE_SERVICE_H_

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

#endif  // MESSENGER_SERVER_DATABASE_DATABASE_SERVICE_H_

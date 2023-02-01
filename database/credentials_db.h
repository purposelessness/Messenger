#ifndef MESSENGER_DATABASE_CREDENTIALS_DB_H_
#define MESSENGER_DATABASE_CREDENTIALS_DB_H_

#include <cstddef>
#include <exception>
#include <optional>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

#include "messenger.pb.h"

class CredentialsDb {
  using Credentials = messenger::Credentials;

 public:
  enum Status {
    kOk,
    kInvalidLogin,
    kInvalidPassword,
    kInvalidId,
    kDuplicateLogin
  };
  struct Responce {
    Status status;
    uint64_t id;
  };

  explicit CredentialsDb(std::string filename);
  void LoadData();
  void PrintData() const;
  void SaveData() const;

  Responce CheckCredentials(const std::string& login,
                            const std::string& password);
  Responce GetUserId(const std::string& login);

  Responce AddUser(const std::string& login, const std::string& password);
  Responce RemoveUser(const std::string& login);
  Responce RemoveUser(uint64_t id);

 private:
  static std::unordered_map<std::string, Credentials> ParseData(
      const std::string& filename);
  static bool IsLoginValid(const std::string& str);

  std::string filename_;
  std::unordered_map<std::string, Credentials> data_;
};

class CredentialsDbException : public std::exception {
 public:
  explicit CredentialsDbException(std::string what);
  [[nodiscard]] const char* what() const noexcept override;

 private:
  std::string what_;
};

#endif  // MESSENGER_DATABASE_CREDENTIALS_DB_H_

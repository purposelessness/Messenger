#ifndef MESSENGER_SERVER_DATABASE_CREDENTIALS_DB_H_
#define MESSENGER_SERVER_DATABASE_CREDENTIALS_DB_H_

#include <exception>
#include <mutex>
#include <optional>
#include <shared_mutex>
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

  Responce CheckCredentials(const std::string& login,
                            const std::string& password);
  Responce GetUserId(const std::string& login);
  Responce AddUser(const std::string& login, const std::string& password);
  void RemoveUser(const std::string& login);

  // Not threadsafe.
  const std::unordered_map<std::string, Credentials>& LoadData();
  void PrintData() const;
  void SaveData() const;

 private:
  static std::unordered_map<std::string, Credentials> ParseData(
      const std::string& filename);
  static bool IsLoginValid(const std::string& str);

  std::string filename_;
  // TODO(purposelessness): self-made map w/o locking
  std::unordered_map<std::string, Credentials> data_;
  mutable std::shared_mutex m_;
};

class CredentialsDbException : public std::exception {
 public:
  explicit CredentialsDbException(std::string what);
  [[nodiscard]] const char* what() const noexcept override;

 private:
  std::string what_;
};

#endif  // MESSENGER_SERVER_DATABASE_CREDENTIALS_DB_H_

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
  struct Responce {
    enum Status {
      kOk,
      kInvalidLogin,
      kInvalidPassword,
      kInvalidId,
      kDuplicateLogin
    } status;
    uint64_t id;
  };

  explicit CredentialsDb(std::string filename = "credentials.data");

  Responce CheckCredentials(const std::string& login,
                            const std::string& password);
  Responce GetUserId(const std::string& login);
  Responce AddUser(const std::string& login, const std::string& password);
  void RemoveUser(const std::string& login);

  // Not threadsafe.
  struct CredentialsResponce {
    enum Status { kOk, kFileNotFound, kParseError } status;
    const std::unordered_map<std::string, Credentials>* data;
  };
  CredentialsResponce LoadData();
  CredentialsResponce::Status SaveData() const;
  void PrintData() const;

 private:
  static bool IsLoginValid(const std::string& str);

  std::string filename_;
  // TODO(purposelessness): self-made map w/o locking
  std::unordered_map<std::string, Credentials> data_;
  mutable std::shared_mutex m_;
};

#endif  // MESSENGER_SERVER_DATABASE_CREDENTIALS_DB_H_

#include "credentials_db.h"

#include <algorithm>
#include <fstream>
#include <future>

CredentialsDb::CredentialsDb(std::string filename)
    : filename_(std::move(filename)) {}

void CredentialsDb::LoadData() { data_ = ParseData(filename_); }

void CredentialsDb::PrintData() const {
  std::for_each(data_.cbegin(), data_.cend(), [](const auto& p) {
    std::cout << "User " << p.second.id() << "\nLogin: " << p.second.login()
              << "\nPassword: " << p.second.password() << '\n';
  });
}

void CredentialsDb::SaveData() const {
  std::ofstream file(filename_, std::ios::trunc | std::ios::binary);
  if (!file.good()) {
    throw CredentialsDbException("File is not found");
  }
  messenger::CredentialBook credential_book;
  std::for_each(data_.begin(), data_.end(),
                [&credential_book](const auto& pair) {
                  auto* entry = credential_book.add_data();
                  entry->set_login(pair.second.login());
                  entry->set_password(pair.second.password());
                  entry->set_id(pair.second.id());
                });

  if (!credential_book.SerializeToOstream(&file)) {
    throw CredentialsDbException("Cannot serialize credential book.");
  }

  google::protobuf::ShutdownProtobufLibrary();
}

CredentialsDb::Responce CredentialsDb::CheckCredentials(
    const std::string& login, const std::string& password) {
  if (!data_.contains(login)) {
    Responce responce{kInvalidLogin, 0};
    return responce;
  }

  auto credentials = data_[login];
  auto out = credentials.password() == password
                 ? Responce{kOk, credentials.id()}
                 : Responce{kInvalidPassword, 0};
  return out;
}

CredentialsDb::Responce CredentialsDb::GetUserId(const std::string& login) {
  auto out = data_.contains(login) ? Responce{kOk, data_[login].id()}
                                   : Responce{kInvalidLogin, 0};
  return out;
}

CredentialsDb::Responce CredentialsDb::AddUser(const std::string& login,
                                               const std::string& password) {
  if (data_.contains(login)) {
    Responce out{kDuplicateLogin, 0};
    return out;
  }

  if (!IsLoginValid(login)) {
    Responce out{kInvalidLogin, 0};
    return out;
  }
  if (!IsLoginValid(password)) {
    Responce out{kInvalidPassword, 0};
    return out;
  }

  auto entry = Credentials{};
  entry.set_login(login);
  entry.set_password(password);
  entry.set_id(data_.size());
  data_[login] = std::move(entry);
  auto out = Responce{kOk, data_[login].id()};
  return out;
}

std::unordered_map<std::string, messenger::Credentials>
CredentialsDb::ParseData(const std::string& filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!file.good()) {
    throw CredentialsDbException("File is not found.");
  }
  messenger::CredentialBook credential_book;
  if (!credential_book.ParseFromIstream(&file)) {
    throw CredentialsDbException("Cannot parse credential book.");
  }

  auto data = credential_book.data();
  std::unordered_map<std::string, Credentials> out;
  out.reserve(data.size());
  std::for_each(data.begin(), data.end(), [&out](const Credentials& c) {
    out.insert(std::pair<std::string, Credentials>{c.login(), c});
  });

  google::protobuf::ShutdownProtobufLibrary();
  return out;
}

bool CredentialsDb::IsLoginValid(const std::string& str) {
  static const std::regex kRegex{R"([\w\!-\/\:-\@\[-\`\{-\~\№]+)"};
  auto out = std::regex_match(str.data(), kRegex);
  return out;
}

CredentialsDbException::CredentialsDbException(std::string what)
    : what_(std::move(what)) {}

const char* CredentialsDbException::what() const noexcept {
  return what_.data();
}

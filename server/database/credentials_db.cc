#include "credentials_db.h"

#include <algorithm>
#include <fstream>
#include <regex>

CredentialsDb::CredentialsDb(std::string filename)
    : filename_(std::move(filename)) {}

CredentialsDb::Responce CredentialsDb::CheckCredentials(
    const std::string& login, const std::string& password) {
  std::shared_lock lk(m_);
  if (!data_.contains(login)) {
    Responce responce{Responce::kInvalidLogin, 0};
    return responce;
  }

  auto credentials = data_[login];
  lk.unlock();
  auto out = credentials.password() == password
                 ? Responce{Responce::kOk, credentials.id()}
                 : Responce{Responce::kInvalidPassword, 0};
  return out;
}

CredentialsDb::Responce CredentialsDb::GetUserId(const std::string& login) {
  std::shared_lock lk(m_);
  auto out = data_.contains(login) ? Responce{Responce::kOk, data_[login].id()}
                                   : Responce{Responce::kInvalidLogin, 0};
  return out;
}

CredentialsDb::Responce CredentialsDb::AddUser(const std::string& login,
                                               const std::string& password) {
  std::shared_lock s_lk(m_);
  if (data_.contains(login)) {
    Responce out{Responce::kDuplicateLogin, 0};
    return out;
  }

  if (!IsLoginValid(login)) {
    Responce out{Responce::kInvalidLogin, 0};
    return out;
  }
  if (!IsLoginValid(password)) {
    Responce out{Responce::kInvalidPassword, 0};
    return out;
  }
  s_lk.unlock();

  auto entry = Credentials{};
  entry.set_login(login);
  entry.set_password(password);
  uint64_t id = 0;
  {
    std::scoped_lock u_lk(m_);
    id = data_.size();
    entry.set_id(id);
    data_[login] = std::move(entry);
  }
  auto out = Responce{Responce::kOk, id};
  return out;
}

void CredentialsDb::RemoveUser(const std::string& login) {
  std::scoped_lock lk(m_);
  data_.erase(login);
}

bool CredentialsDb::IsLoginValid(const std::string& str) {
  static const std::regex kRegex{R"([\w\!-\/\:-\@\[-\`\{-\~\№]+)"};
  auto out = std::regex_match(str.data(), kRegex);
  return out;
}

CredentialsDb::CredentialsResponce CredentialsDb::LoadData() {
  std::ifstream file(filename_, std::ios::binary);
  if (!file.good()) {
    return {CredentialsResponce::kFileNotFound, nullptr};
  }
  messenger::CredentialBook credential_book;
  if (!credential_book.ParseFromIstream(&file)) {
    return {CredentialsResponce::kParseError, nullptr};
  }

  auto data = credential_book.data();
  std::unordered_map<std::string, Credentials> data_map;
  data_map.reserve(data.size());
  std::for_each(std::make_move_iterator(data.begin()),
                std::make_move_iterator(data.end()),
                [&data_map](Credentials&& c) {
                  data_map.emplace(c.login(), std::move(c));
                });

  google::protobuf::ShutdownProtobufLibrary();
  data_ = std::move(data_map);
  return {CredentialsResponce::kOk, &data_};
}

void CredentialsDb::PrintData() const {
  std::shared_lock lk(m_);
  std::for_each(data_.cbegin(), data_.cend(), [](const auto& p) {
    std::cout << "User " << p.second.id() << "\nLogin: " << p.second.login()
              << "\nPassword: " << p.second.password() << '\n';
  });
}

CredentialsDb::CredentialsResponce::Status CredentialsDb::SaveData() const {
  std::ofstream file(filename_, std::ios::trunc | std::ios::binary);
  if (!file.good()) {
    std::cerr << "Credentials db: file not found.\n";
    return CredentialsResponce::kFileNotFound;
  }
  messenger::CredentialBook credential_book;
  std::for_each(data_.begin(), data_.end(),
                [&credential_book](const auto& pair) {
                  auto* entry = credential_book.add_data();
                  *entry = pair.second;
                });

  if (!credential_book.SerializeToOstream(&file)) {
    std::cerr << "Credentials db: cannot serialize data.\n";
    return CredentialsResponce::kParseError;
  }

  google::protobuf::ShutdownProtobufLibrary();
  std::cout << "Credentials db: data is saved.\n";
  return CredentialsResponce::kOk;
}

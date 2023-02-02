#include "details_db.h"

void DetailsDb::AddId(const std::string& login, uint64_t id) {
  id_data_.Insert(login, id);
  login_data_.Insert(id, login);
}

void DetailsDb::RemoveId(const std::string& login) {
  auto id = id_data_[login];
  if (id.has_value()) {
    id_data_.Erase(login);
    login_data_.Erase(id.value());
  }
}

std::optional<uint64_t> DetailsDb::GetId(const std::string& login) {
  return id_data_[login];
}

std::optional<std::string> DetailsDb::GetLogin(uint64_t id) {
  return login_data_[id];
}

void DetailsDb::AddActive(uint64_t id) { active_data_.Insert(id); }

void DetailsDb::RemoveActive(uint64_t id) { active_data_.Erase(id); }

bool DetailsDb::IsActive(uint64_t id) { return active_data_.Contains(id); }

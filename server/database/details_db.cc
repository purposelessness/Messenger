#include "details_db.h"

void DetailsDb::AddId(const std::string& login, uint64_t id) {
  id_data_.Insert(login, id);
}

void DetailsDb::RemoveId(const std::string& login) {
  id_data_.Erase(login);
}

std::optional<uint64_t> DetailsDb::GetId(const std::string& login) {
  auto out = id_data_[login];
  return out;
}

void DetailsDb::AddActive(uint64_t id) {
  active_data_.Insert(id);
}

void DetailsDb::RemoveActive(uint64_t id) {
  active_data_.Erase(id);
}

bool DetailsDb::IsActive(uint64_t id) {
  return active_data_.Contains(id);
}

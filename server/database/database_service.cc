#include "database_service.h"

DatabaseService::DatabaseService(std::string credentials_db_name)
    : credentials_db_name_(std::move(credentials_db_name)) {}

std::optional<uint64_t> DatabaseService::CheckCredentials(
    const std::string& login, const std::string& password) {
  
}

#include <gtest/gtest.h>

#include "../database/credentials_db.h"

class CredentialsDbTest : public ::testing::Test {
 public:
  CredentialsDbTest() : db("credentials.data") {}

 protected:
  void SetUp() override {
    try {
      db.LoadData();
    } catch (const CredentialsDbException& e) {
      std::cerr << "Exception: " << e.what() << '\n';
    }
    db.PrintData();
  }

  void TearDown() override {
    try {
      db.SaveData();
    } catch (const CredentialsDbException& e) {
      std::cerr << "Exception: " << e.what() << '\n';
    }
  }

  CredentialsDb db;
};

TEST_F(CredentialsDbTest, AddUser) {
  auto responce_1 = db.AddUser("some invalid", "idk");
  EXPECT_EQ(responce_1.status, CredentialsDb::kInvalidLogin);
  auto responce_2 = db.AddUser("ok", "some invalid");
  EXPECT_EQ(responce_2.status, CredentialsDb::kInvalidPassword);
  auto responce_3 = db.AddUser("aboba", "aboba");
  EXPECT_EQ(responce_3.status, CredentialsDb::kOk);
  auto responce_4 = db.AddUser("aboba", "zeleboba");
  EXPECT_EQ(responce_4.status, CredentialsDb::kDuplicateLogin);
  auto responce_5 = db.AddUser("syrtce", "dassyr");
  EXPECT_EQ(responce_5.status, CredentialsDb::kOk);
  auto responce_6 = db.AddUser("zxc_123@rambler.com", "!@#$%^&*()");
  EXPECT_EQ(responce_6.status, CredentialsDb::kOk);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

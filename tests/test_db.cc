#include <gtest/gtest.h>

#include "../server/database/credentials_db.h"

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

TEST_F(CredentialsDbTest, MainTest) {
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

  auto responce_7 = db.GetUserId("aboba");
  EXPECT_EQ(responce_7.status, CredentialsDb::kOk);
  EXPECT_EQ(responce_7.id, 0);
  auto responce_8 = db.CheckCredentials("aboba", "aboba");
  EXPECT_EQ(responce_8.status, CredentialsDb::kOk);
  EXPECT_EQ(responce_8.id, 0);
  auto responce_9 = db.RemoveUser("aboba");
  EXPECT_EQ(responce_9.status, CredentialsDb::kOk);
  auto responce_10 = db.GetUserId("aboba");
  EXPECT_EQ(responce_10.status, CredentialsDb::kInvalidLogin);
  auto responce_11 = db.CheckCredentials("syrtce", "??");
  EXPECT_EQ(responce_11.status, CredentialsDb::kInvalidPassword);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

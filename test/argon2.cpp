#include <string>
#include <vector>

#include <argon2/argon2.h>
#include <gtest/gtest.h>

class Argon2Test : public ::testing::Test
{
protected:
  static constexpr size_t HASHLEN = 32;
  static constexpr size_t SALTLEN = 16;
  static constexpr const char* PWD = "password";

  std::vector<uint8_t> hash1;
  std::vector<uint8_t> hash2;
  std::vector<uint8_t> salt;
  std::vector<uint8_t> pwd;
  uint32_t pwdlen;

  void SetUp() override
  {
    hash1.resize(HASHLEN);
    hash2.resize(HASHLEN);
    salt.assign(SALTLEN, 0x00);

    pwdlen = strlen(PWD);
    pwd.assign(PWD, PWD + pwdlen);
  }
};

TEST_F(Argon2Test, HighLevelAndLowLevelApisProduceSameHash)
{
  uint32_t t_cost = 2;         // 2-pass computation
  uint32_t m_cost = (1 << 10); // Reduced for faster tests (1 MiB)
  uint32_t parallelism = 1;    // number of threads and lanes

  // Test high-level API
  int rc = argon2i_hash_raw(t_cost,
                            m_cost,
                            parallelism,
                            pwd.data(),
                            pwdlen,
                            salt.data(),
                            salt.size(),
                            hash1.data(),
                            hash1.size());
  ASSERT_EQ(ARGON2_OK, rc) << "High-level API failed: "
                           << argon2_error_message(rc);

  // Test low-level API
  argon2_context context = {
    hash2.data(),                        // output array
    static_cast<uint32_t>(hash2.size()), // output length
    pwd.data(),                          // password array
    pwdlen,                              // password length
    salt.data(),                         // salt array
    static_cast<uint32_t>(salt.size()),  // salt length
    nullptr,
    0, // secret data
    nullptr,
    0,                 // associated data
    t_cost,            // time cost
    m_cost,            // memory cost
    parallelism,       // lanes
    parallelism,       // threads
    ARGON2_VERSION_13, // algorithm version
    nullptr,           // custom memory allocation
    nullptr,           // custom deallocation
    ARGON2_DEFAULT_FLAGS};

  rc = argon2i_ctx(&context);
  ASSERT_EQ(ARGON2_OK, rc) << "Low-level API failed: "
                           << argon2_error_message(rc);

  // Compare results from both APIs
  EXPECT_EQ(hash1, hash2)
    << "High-level and low-level APIs produced different hashes";
}

TEST_F(Argon2Test, DifferentPasswordsProduceDifferentHashes)
{
  uint32_t t_cost = 2;
  uint32_t m_cost = (1 << 10);
  uint32_t parallelism = 1;

  std::vector<uint8_t> other_pwd = {
    'd', 'i', 'f', 'f', 'e', 'r', 'e', 'n', 't'};
  std::vector<uint8_t> other_hash(HASHLEN);

  // Hash with original password
  int rc = argon2i_hash_raw(t_cost,
                            m_cost,
                            parallelism,
                            pwd.data(),
                            pwdlen,
                            salt.data(),
                            salt.size(),
                            hash1.data(),
                            hash1.size());
  ASSERT_EQ(ARGON2_OK, rc);

  // Hash with different password
  rc = argon2i_hash_raw(t_cost,
                        m_cost,
                        parallelism,
                        other_pwd.data(),
                        other_pwd.size(),
                        salt.data(),
                        salt.size(),
                        other_hash.data(),
                        other_hash.size());
  ASSERT_EQ(ARGON2_OK, rc);

  // Hashes should be different
  EXPECT_NE(hash1, other_hash) << "Different passwords produced the same hash";
}

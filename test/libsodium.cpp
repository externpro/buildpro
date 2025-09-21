#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <sodium.h>

class LibsodiumTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Initialize libsodium
    if (sodium_init() < 0)
    {
      FAIL() << "Failed to initialize libsodium";
    }
  }

  // Helper function to convert binary data to hex string
  std::string bin2hex(const std::vector<unsigned char>& bin)
  {
    std::string hex(bin.size() * 2 + 1, '\0');
    char* result = sodium_bin2hex(
      const_cast<char*>(hex.data()), hex.size(), bin.data(), bin.size());
    if (result == nullptr)
    {
      return "";
    }
    hex.resize(hex.find('\0'));
    return hex;
  }
};

// Test basic random number generation
TEST_F(LibsodiumTest, RandomNumberGeneration)
{
  unsigned char random1[32], random2[32];
  randombytes_buf(random1, sizeof(random1));
  randombytes_buf(random2, sizeof(random2));

  // Very small chance this could fail randomly, but extremely unlikely
  EXPECT_NE(0, memcmp(random1, random2, sizeof(random1)));
}

// Test hashing with SHA-256
TEST_F(LibsodiumTest, Hashing)
{
  const std::string message = "Hello, libsodium!";
  unsigned char hash[crypto_hash_sha256_BYTES];

  crypto_hash_sha256(hash,
                     reinterpret_cast<const unsigned char*>(message.data()),
                     message.size());

  // Verify the hash is not all zeros
  bool all_zeros = true;
  for (size_t i = 0; i < sizeof(hash); ++i)
  {
    if (hash[i] != 0)
    {
      all_zeros = false;
      break;
    }
  }
  EXPECT_FALSE(all_zeros);
}

// Test authenticated encryption with secret key
TEST_F(LibsodiumTest, SecretKeyEncryption)
{
  const std::string plaintext = "This is a secret message";

  // Generate a random key
  unsigned char key[crypto_secretbox_KEYBYTES];
  unsigned char nonce[crypto_secretbox_NONCEBYTES];
  randombytes_buf(key, sizeof(key));
  randombytes_buf(nonce, sizeof(nonce));

  // Encrypt
  std::vector<unsigned char> ciphertext(plaintext.size() +
                                        crypto_secretbox_MACBYTES);
  crypto_secretbox_easy(
    ciphertext.data(),
    reinterpret_cast<const unsigned char*>(plaintext.data()),
    plaintext.size(),
    nonce,
    key);

  // Decrypt
  std::vector<unsigned char> decrypted(plaintext.size());
  if (crypto_secretbox_open_easy(
        decrypted.data(), ciphertext.data(), ciphertext.size(), nonce, key) !=
      0)
  {
    FAIL() << "Decryption failed";
  }

  // Verify
  std::string result(
    reinterpret_cast<const char*>(decrypted.data()), decrypted.size());
  EXPECT_EQ(plaintext, result);
}

// Test public key encryption
TEST_F(LibsodiumTest, PublicKeyEncryption)
{
  const std::string message = "This is a public-key encrypted message";

  // Generate key pair
  unsigned char public_key[crypto_box_PUBLICKEYBYTES];
  unsigned char secret_key[crypto_box_SECRETKEYBYTES];
  crypto_box_keypair(public_key, secret_key);

  // Generate nonce
  unsigned char nonce[crypto_box_NONCEBYTES];
  randombytes_buf(nonce, sizeof(nonce));

  // Encrypt (in a real scenario, you'd use the recipient's public key)
  std::vector<unsigned char> ciphertext(message.size() + crypto_box_MACBYTES);
  if (crypto_box_easy(
        ciphertext.data(),
        reinterpret_cast<const unsigned char*>(message.data()),
        message.size(),
        nonce,
        public_key, // In real use, this would be the recipient's public key
        secret_key) != 0)
  { // And this would be the sender's secret key
    FAIL() << "Public key encryption failed";
  }

  // Decrypt (using the same key pair for testing)
  std::vector<unsigned char> decrypted(message.size());
  if (crypto_box_open_easy(decrypted.data(),
                           ciphertext.data(),
                           ciphertext.size(),
                           nonce,
                           public_key, // Sender's public key
                           secret_key) != 0)
  { // Recipient's secret key
    FAIL() << "Public key decryption failed";
  }

  // Verify
  std::string result(
    reinterpret_cast<const char*>(decrypted.data()), decrypted.size());
  EXPECT_EQ(message, result);
}

// Test password hashing (Argon2)
TEST_F(LibsodiumTest, PasswordHashing)
{
  const std::string password = "my_super_secret_password";

  // Generate random salt
  std::vector<unsigned char> salt(crypto_pwhash_SALTBYTES);
  randombytes_buf(salt.data(), salt.size());

  // Hash the password
  std::vector<unsigned char> hash(crypto_pwhash_STRBYTES);
  if (crypto_pwhash_str(reinterpret_cast<char*>(hash.data()),
                        password.c_str(),
                        password.size(),
                        crypto_pwhash_OPSLIMIT_MODERATE,
                        crypto_pwhash_MEMLIMIT_MODERATE) != 0)
  {
    FAIL() << "Password hashing failed";
  }

  // Verify the password
  EXPECT_EQ(0,
            crypto_pwhash_str_verify(reinterpret_cast<const char*>(hash.data()),
                                     password.c_str(),
                                     password.size()));

  // Test with wrong password
  EXPECT_NE(
    0,
    crypto_pwhash_str_verify(
      reinterpret_cast<const char*>(hash.data()), "wrong_password", 13));
}

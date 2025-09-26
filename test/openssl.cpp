#include <iostream>
#include <string>

#include <gtest/gtest.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/ssl.h>

class OpenSSLTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Initialize OpenSSL
    OPENSSL_init_ssl(0, NULL);
    OPENSSL_init_crypto(OPENSSL_INIT_LOAD_CRYPTO_STRINGS |
                          OPENSSL_INIT_ADD_ALL_CIPHERS |
                          OPENSSL_INIT_ADD_ALL_DIGESTS,
                        NULL);
  }

  void TearDown() override
  {
    // Modern OpenSSL (1.1.0+) handles cleanup automatically
  }

  std::string sha256(const std::string& data)
  {
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;

    EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(mdctx, data.c_str(), data.length());
    EVP_DigestFinal_ex(mdctx, hash, &hash_len);
    EVP_MD_CTX_free(mdctx);

    return std::string(reinterpret_cast<char*>(hash), hash_len);
  }

  std::string base64Encode(const std::string& data)
  {
    BIO *bio, *b64;
    BUF_MEM* bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_write(bio, data.c_str(), data.length());
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);

    std::string result(bufferPtr->data, bufferPtr->length);

    BIO_free_all(bio);
    return result;
  }
};

TEST_F(OpenSSLTest, VersionInfo)
{
  // Test that we can get the OpenSSL version
  const char* version = OpenSSL_version(OPENSSL_VERSION);
  ASSERT_NE(version, nullptr) << "Failed to get OpenSSL version";

  std::cout << "Using OpenSSL version: " << version << std::endl;

  // Check that the version is at least 1.1.0
  long version_num = OpenSSL_version_num();
  int major = (version_num >> 28) & 0xFF;
  int minor = (version_num >> 20) & 0xFF;
  int fix = (version_num >> 12) & 0xFF;

  std::cout << "Version number: 0x" << std::hex << version_num << std::dec
            << " (" << major << "." << minor << "." << fix << ")" << std::endl;

  EXPECT_GE(major, 1) << "Unexpected major version";
  EXPECT_GE(minor, 1) << "OpenSSL version too old, need at least 1.1.0";
}

TEST_F(OpenSSLTest, RandomNumberGeneration)
{
  // Test random number generation
  unsigned char buffer[32];
  int rc = RAND_bytes(buffer, sizeof(buffer));

  EXPECT_EQ(rc, 1) << "Failed to generate random bytes: "
                   << ERR_error_string(ERR_get_error(), NULL);

  // Check that we didn't get all zeros (very unlikely)
  bool allZeros = true;
  for (size_t i = 0; i < sizeof(buffer); i++)
  {
    if (buffer[i] != 0)
    {
      allZeros = false;
      break;
    }
  }
  EXPECT_FALSE(allZeros) << "Random number generator returned all zeros";
}

TEST_F(OpenSSLTest, Hashing)
{
  // Test SHA-256 hashing
  std::string testData = "Hello, OpenSSL!";
  std::string hash = sha256(testData);

  // The hash should be 32 bytes (256 bits) long
  EXPECT_EQ(hash.length(), 32) << "Unexpected hash length";

  // Test with empty string
  std::string emptyHash = sha256("");
  EXPECT_EQ(emptyHash.length(), 32) << "Empty string hash has wrong length";
}

TEST_F(OpenSSLTest, Base64Encoding)
{
  // Test Base64 encoding/decoding
  std::string testData = "This is a test string for Base64 encoding.";
  std::string encoded = base64Encode(testData);

  // Decode back
  BIO *bio, *b64;
  char* buffer = (char*)malloc(testData.length());

  b64 = BIO_new(BIO_f_base64());
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
  bio = BIO_new_mem_buf(encoded.c_str(), encoded.length());
  bio = BIO_push(b64, bio);

  int decodedLength = BIO_read(bio, buffer, testData.length());
  BIO_free_all(bio);

  std::string decoded(buffer, decodedLength);
  free(buffer);

  EXPECT_EQ(decoded, testData) << "Base64 decode(encode(x)) != x";
}

TEST_F(OpenSSLTest, RSAKeyGeneration)
{
  // Test RSA key generation
  EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
  ASSERT_NE(ctx, nullptr) << "Failed to create EVP_PKEY_CTX";

  int rc = EVP_PKEY_keygen_init(ctx);
  ASSERT_EQ(rc, 1) << "Failed to initialize key generation";

  // Set key parameters (2048-bit key)
  rc = EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048);
  ASSERT_EQ(rc, 1) << "Failed to set RSA key length";

  // Generate the key
  EVP_PKEY* pkey = nullptr;
  rc = EVP_PKEY_keygen(ctx, &pkey);
  ASSERT_EQ(rc, 1) << "Failed to generate RSA key";

  // Check key properties
  RSA* rsa = EVP_PKEY_get0_RSA(pkey);
  ASSERT_NE(rsa, nullptr) << "Failed to get RSA key";

  const BIGNUM* n = RSA_get0_n(rsa);
  const BIGNUM* e = RSA_get0_e(rsa);
  const BIGNUM* d = RSA_get0_d(rsa);

  EXPECT_NE(n, nullptr) << "RSA modulus is null";
  EXPECT_NE(e, nullptr) << "RSA public exponent is null";
  EXPECT_NE(d, nullptr) << "RSA private exponent is null";

  // Check key size
  int keySize = RSA_size(rsa);
  EXPECT_GE(keySize, 256) << "RSA key size too small";

  // Clean up
  EVP_PKEY_free(pkey);
  EVP_PKEY_CTX_free(ctx);
}

TEST_F(OpenSSLTest, HMAC)
{
  // Test HMAC-SHA256
  std::string key = "secret-key";
  std::string data = "Hello, HMAC!";

  unsigned char hmac[EVP_MAX_MD_SIZE];
  unsigned int hmac_len;

  HMAC(EVP_sha256(),
       key.c_str(),
       key.length(),
       reinterpret_cast<const unsigned char*>(data.c_str()),
       data.length(),
       hmac,
       &hmac_len);

  EXPECT_GT(hmac_len, 0) << "HMAC generation failed";

  // Convert to hex for output
  std::string hmac_hex;
  for (unsigned int i = 0; i < hmac_len; i++)
  {
    char buf[3];
    snprintf(buf, sizeof(buf), "%02x", hmac[i]);
    hmac_hex += buf;
  }

  std::cout << "HMAC-SHA256: " << hmac_hex << std::endl;
}

#include <iostream>

#include <gtest/gtest.h>
#include <libssh2.h>

// Simple test class for basic libssh2 functionality
class LibSsh2Test : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Initialize libssh2
    int rc = libssh2_init(0);
    ASSERT_EQ(0, rc) << "Failed to initialize libssh2";
  }

  void TearDown() override
  {
    // Cleanup libssh2
    libssh2_exit();
  }
};

// Test basic library initialization and version checking
TEST_F(LibSsh2Test, LibraryInitialization)
{
  // Test that we can get the library version
  const char* version = libssh2_version(LIBSSH2_VERSION_NUM);
  ASSERT_NE(nullptr, version) << "Failed to get libssh2 version";
  std::cout << "Using libssh2 version: " << version << std::endl;

  // Verify we can create a session
  LIBSSH2_SESSION* session = libssh2_session_init();
  ASSERT_NE(nullptr, session) << "Failed to create SSH session";

  // Clean up the session
  libssh2_session_free(session);
}

// Test session options
TEST_F(LibSsh2Test, SessionOptions)
{
  // Create a session
  LIBSSH2_SESSION* session = libssh2_session_init();
  ASSERT_NE(nullptr, session) << "Failed to create SSH session";

  // Test setting and getting timeout
  long timeout = 5000; // 5 seconds
  libssh2_session_set_timeout(session, timeout);
  long retrieved_timeout = libssh2_session_get_timeout(session);
  EXPECT_EQ(timeout, retrieved_timeout) << "Session timeout not set correctly";

  // Test blocking mode
  libssh2_session_set_blocking(session, 0); // Non-blocking
  int blocking = libssh2_session_get_blocking(session);
  EXPECT_EQ(0, blocking) << "Session blocking mode not set correctly";

  libssh2_session_set_blocking(session, 1); // Blocking
  blocking = libssh2_session_get_blocking(session);
  EXPECT_EQ(1, blocking) << "Session blocking mode not set correctly";

  // Clean up
  libssh2_session_free(session);
}

// Test error handling
TEST_F(LibSsh2Test, ErrorHandling)
{
  // Create a session
  LIBSSH2_SESSION* session = libssh2_session_init();
  ASSERT_NE(nullptr, session) << "Failed to create SSH session";

  // Test getting last error (should be no error initially)
  char* errmsg = nullptr;
  int errlen = 0;
  int errcode = libssh2_session_last_error(session, &errmsg, &errlen, 0);

  // Just verify we can call the error functions without crashing
  std::cout << "Initial error code: " << errcode << std::endl;
  if (errmsg)
  {
    std::cout << "Error message: " << errmsg << std::endl;
  }

  // Clean up
  libssh2_session_free(session);
}

#include <iostream>
#include <string>

// Include GLEW header but don't use any functions that require an OpenGL
// context
#include <GL/glew.h>

#include <gtest/gtest.h>

// Simple test to verify GLEW library is linked correctly
TEST(GlewTest, LibraryPresence)
{
  // We can safely check the GLEW version string without an OpenGL context
  const GLubyte* version = glewGetString(GLEW_VERSION);

  // Just verify we got a version string
  ASSERT_NE(version, nullptr) << "Failed to get GLEW version string";
  std::cout << "GLEW version: " << version << std::endl;

  // Verify the version string format (should be like "2.1.0" or similar)
  std::string versionStr(reinterpret_cast<const char*>(version));
  EXPECT_FALSE(versionStr.empty()) << "GLEW version string is empty";

  std::cout
    << "Note: Full GLEW functionality tests are skipped because they require"
    << std::endl;
  std::cout << "an active OpenGL context, which is not available in the test "
               "environment."
            << std::endl;
  std::cout
    << "This test only verifies that the GLEW library is properly linked."
    << std::endl;
}

// Test that we can access GLEW's error string function
TEST(GlewTest, ErrorStringFunction)
{
  // We can safely call glewGetErrorString without an OpenGL context
  const GLubyte* errorStr = glewGetErrorString(GLEW_OK);
  ASSERT_NE(errorStr, nullptr) << "Failed to get GLEW error string";
  std::cout << "GLEW_OK error string: " << errorStr << std::endl;

  // Check error string for a different error code
  const GLubyte* errorStr2 = glewGetErrorString(GLEW_ERROR_NO_GL_VERSION);
  ASSERT_NE(errorStr2, nullptr)
    << "Failed to get GLEW error string for GLEW_ERROR_NO_GL_VERSION";
  std::cout << "GLEW_ERROR_NO_GL_VERSION error string: " << errorStr2
            << std::endl;
}

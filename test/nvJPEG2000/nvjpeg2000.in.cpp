#include <gtest/gtest.h>
#include <nvJPEG2000/nvjpeg2k_version.h>

TEST(NvJpeg2000VersionTest, CheckVersion)
{
  // Compare the version from the header with the expected version
  // clang-format off
  EXPECT_EQ(NVJPEG2K_VER_MAJOR, @VERSION_MAJOR@);
  EXPECT_EQ(NVJPEG2K_VER_MINOR, @VERSION_MINOR@);
  EXPECT_EQ(NVJPEG2K_VER_PATCH, @VERSION_PATCH@);
  // clang-format on

  // Print the actual version for reference
  printf("nvJPEG2000 Version: %d.%d.%d\n",
         NVJPEG2K_VER_MAJOR,
         NVJPEG2K_VER_MINOR,
         NVJPEG2K_VER_PATCH);
}

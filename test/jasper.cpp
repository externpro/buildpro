#include <cstring>
#include <iostream>
#include <vector>

#include <gtest/gtest.h>
#include <jasper/jasper.h>

class JasperTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Initialize the Jasper library
    if (jas_init() != 0)
    {
      std::cerr << "Failed to initialize Jasper library" << std::endl;
    }
  }

  void TearDown() override
  {
    // Clean up Jasper library resources
    jas_cleanup();
  }
};

// Test Jasper version information
TEST_F(JasperTest, Version)
{
  // Get the Jasper version string
  const char* version = jas_getversion();

  // Print version information
  std::cout << "Jasper version: " << version << std::endl;

  // Verify version is valid
  EXPECT_NE(nullptr, version);
  EXPECT_STRNE("", version);
}

// Test Jasper initialization and cleanup
TEST_F(JasperTest, InitAndCleanup)
{
  // Jasper is already initialized in SetUp, so we'll clean up and re-initialize
  jas_cleanup();

  // Re-initialize and check result
  int result = jas_init();
  EXPECT_EQ(0, result) << "Jasper initialization failed";
}

// Test Jasper image format information
TEST_F(JasperTest, ImageFormats)
{
  // Check for JP2 format specifically (JPEG 2000)
  int jp2FormatID = jas_image_strtofmt(const_cast<char*>("jp2"));
  EXPECT_GE(jp2FormatID, 0) << "JPEG 2000 format not supported";

  std::cout << "JP2 format ID: " << jp2FormatID << std::endl;
}

// Test Jasper stream operations
TEST_F(JasperTest, StreamOperations)
{
  // Create a memory buffer
  const char* testData = "Jasper Test Data";
  size_t testDataLen = strlen(testData);

  // Open memory stream for writing
  jas_stream_t* stream = jas_stream_memopen(nullptr, 0);
  ASSERT_NE(nullptr, stream) << "Failed to open memory stream";

  // Write data to stream
  size_t bytesWritten = jas_stream_write(stream, testData, testDataLen);
  EXPECT_EQ(testDataLen, bytesWritten) << "Failed to write all data to stream";

  // Rewind stream
  jas_stream_rewind(stream);

  // Read data back
  std::vector<char> buffer(testDataLen + 1, 0);
  size_t bytesRead = jas_stream_read(stream, buffer.data(), testDataLen);
  EXPECT_EQ(testDataLen, bytesRead) << "Failed to read all data from stream";

  // Verify data
  EXPECT_STREQ(testData, buffer.data())
    << "Data read from stream doesn't match written data";

  // Close stream
  jas_stream_close(stream);
}

// Test Jasper color management
TEST_F(JasperTest, ColorManagement)
{
  // Test color space creation
  jas_cmprof_t* profile = jas_cmprof_createfromclrspc(JAS_CLRSPC_SRGB);
  ASSERT_NE(nullptr, profile) << "Failed to create color profile for sRGB";

  // Clean up
  jas_cmprof_destroy(profile);
}

// Test creating a simple image
TEST_F(JasperTest, CreateImage)
{
  // Create a small grayscale image (8x8 pixels)
  const int width = 8;
  const int height = 8;
  const int depth = 8;         // 8-bit
  const int numComponents = 1; // grayscale

  // Set up component parameters
  jas_image_cmptparm_t cmptparm;
  memset(&cmptparm, 0, sizeof(jas_image_cmptparm_t));
  cmptparm.tlx = 0;
  cmptparm.tly = 0;
  cmptparm.hstep = 1;
  cmptparm.vstep = 1;
  cmptparm.width = width;
  cmptparm.height = height;
  cmptparm.prec = depth;
  cmptparm.sgnd = false;

  // Create the image with one component
  jas_image_t* image =
    jas_image_create(numComponents, &cmptparm, JAS_CLRSPC_SGRAY);
  ASSERT_NE(nullptr, image) << "Failed to create image";

  // Set some pixel values
  for (int y = 0; y < height; ++y)
  {
    for (int x = 0; x < width; ++x)
    {
      // Create a simple checkerboard pattern
      int value = ((x + y) % 2) * 255;
      jas_image_writecmptsample(image, 0, x, y, value);
    }
  }

  // Verify some pixel values
  for (int y = 0; y < height; y += 2)
  {
    for (int x = 0; x < width; x += 2)
    {
      int expected = ((x + y) % 2) * 255;
      int actual = jas_image_readcmptsample(image, 0, x, y);
      EXPECT_EQ(expected, actual)
        << "Pixel value mismatch at (" << x << ", " << y << ")";
    }
  }

  // Clean up
  jas_image_destroy(image);
}

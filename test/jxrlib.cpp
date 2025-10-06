#include <iostream>

#include <gtest/gtest.h>
#include <jxrlib/JXRGlue.h>

class JxrlibTest : public ::testing::Test
{
protected:
  void SetUp() override { }
  void TearDown() override { }
};

// Test SDK version information
TEST_F(JxrlibTest, SdkVersion)
{
  // Print SDK version for reference
  std::cout << "JPEG XR SDK version: " << WMP_SDK_VERSION << std::endl;
  EXPECT_GT(WMP_SDK_VERSION, 0U) << "SDK version should be greater than 0";
}

// Test that we can create a codec factory
TEST_F(JxrlibTest, CodecFactoryCreation)
{
  PKCodecFactory* pFactory = NULL;
  ERR err = WMP_errSuccess;

  // Create codec factory
  err = PKCreateCodecFactory(&pFactory, WMP_SDK_VERSION);
  ASSERT_EQ(WMP_errSuccess, err) << "Failed to create codec factory";
  ASSERT_NE(nullptr, pFactory) << "Codec factory is null";

  // Clean up
  if (pFactory)
    pFactory->Release(&pFactory);
}

// Test pixel format constants
TEST_F(JxrlibTest, PixelFormatConstants)
{
  // Verify that pixel format GUIDs are available
  EXPECT_NE(GUID_PKPixelFormat24bppRGB.Data1, 0)
    << "24bppRGB format GUID is invalid";
  EXPECT_NE(GUID_PKPixelFormat32bppRGBA.Data1, 0)
    << "32bppRGBA format GUID is invalid";

  // Print some pixel format information
  std::cout << "24bppRGB GUID: " << GUID_PKPixelFormat24bppRGB.Data1
            << std::endl;
  std::cout << "32bppRGBA GUID: " << GUID_PKPixelFormat32bppRGBA.Data1
            << std::endl;
}

// Test error codes
TEST_F(JxrlibTest, ErrorCodes)
{
  // Verify that error codes are defined correctly
  EXPECT_EQ(WMP_errSuccess, 0) << "WMP_errSuccess should be 0";
  EXPECT_NE(WMP_errFail, WMP_errSuccess)
    << "WMP_errFail should not equal WMP_errSuccess";

  // Print error code values
  std::cout << "WMP_errSuccess: " << WMP_errSuccess << std::endl;
  std::cout << "WMP_errFail: " << WMP_errFail << std::endl;
}

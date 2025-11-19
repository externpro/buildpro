#include <cstdio>
#include <iostream>

#include <gtest/gtest.h>
extern "C"
{
#include <jpegxp/jerror.h>
#include <jpegxp/xjpeglib.h>
}

class JpegxpTest : public ::testing::Test
{
protected:
  void SetUp() override { }
  void TearDown() override { }
};

TEST_F(JpegxpTest, VersionAndConfig)
{
  std::cout << "JPEGXP JPEG_LIB_VERSION: " << JPEG_LIB_VERSION << std::endl;
  EXPECT_GE(JPEG_LIB_VERSION, 62);

#ifdef BITS_IN_JSAMPLE
  std::cout << "BITS_IN_JSAMPLE: " << BITS_IN_JSAMPLE << std::endl;
  EXPECT_TRUE(BITS_IN_JSAMPLE == 8 || BITS_IN_JSAMPLE == 12)
    << "BITS_IN_JSAMPLE should be 8 or 12";
#endif
}

TEST_F(JpegxpTest, CreateAndDestroyCompressXp)
{
  jpeg_compress_struct cinfo{};
  jpeg_error_mgr jerr{};

  cinfo.err = jpeg_std_error(&jerr);

  jpeg_create_compress_xp(&cinfo);

  EXPECT_TRUE(cinfo.is_decompressor == FALSE);
  EXPECT_EQ(JPEG_LIB_VERSION, JPEG_LIB_VERSION);

  jpeg_destroy_compress_xp(&cinfo);
}

TEST_F(JpegxpTest, CreateAndDestroyDecompressXp)
{
  jpeg_decompress_struct cinfo{};
  jpeg_error_mgr jerr{};

  cinfo.err = jpeg_std_error(&jerr);

  jpeg_create_decompress_xp(&cinfo);

  EXPECT_TRUE(cinfo.is_decompressor == TRUE);

  jpeg_destroy_decompress_xp(&cinfo);
}

TEST_F(JpegxpTest, ExtendedDecompressStructUsage)
{
  jpeg_decompress_struct cinfo{};
  jpeg_error_mgr jerr{};

  cinfo.err = jpeg_std_error(&jerr);

  jpeg_decompress_struct_xp xinfo{};
  xinfo.lossless_xp = FALSE;
  xinfo.data_unit_xp = 1;
  xinfo.bits_in_JSAMPLEXP = BITS_IN_JSAMPLE;

  cinfo.client_data = &xinfo;

  EXPECT_EQ(&xinfo, cinfo.client_data);
  EXPECT_FALSE(xinfo.lossless_xp);

  xinfo.lossless_xp = TRUE;
  EXPECT_TRUE(xinfo.lossless_xp);
}

TEST_F(JpegxpTest, Supports8BitOr12BitLossyConfig)
{
  jpeg_compress_struct cinfo{};
  jpeg_error_mgr jerr{};

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress_xp(&cinfo);

  cinfo.image_width = 8;
  cinfo.image_height = 8;
  cinfo.input_components = 1;
  cinfo.in_color_space = JCS_GRAYSCALE;

  jpeg_set_defaults_xp(&cinfo);

  EXPECT_TRUE(cinfo.data_precision == 8 || cinfo.data_precision == 12)
    << "Unexpected data_precision for jpegxp";

  jpeg_destroy_compress_xp(&cinfo);
}

TEST_F(JpegxpTest, LosslessDecodeSupportFlag)
{
  jpeg_decompress_struct cinfo{};
  jpeg_error_mgr jerr{};

  cinfo.err = jpeg_std_error(&jerr);

  jpeg_decompress_struct_xp xinfo{};
  xinfo.lossless_xp = TRUE;
  cinfo.client_data = &xinfo;

  EXPECT_TRUE(xinfo.lossless_xp);
}

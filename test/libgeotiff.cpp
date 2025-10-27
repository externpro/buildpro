#include <cstdio>
#include <cstring>

#include <gtest/gtest.h>
#include <libgeotiff/geotiffio.h>
#include <libgeotiff/xtiffio.h>

// Simple test to verify basic libgeotiff functionality
TEST(LibGeoTIFFTest, BasicFunctionality)
{
  // Use a fixed test file in the current directory
  const char* test_file = "simple_test.tif";

  // Clean up any existing test file
  std::remove(test_file);

  // 1. Create a simple TIFF file
  TIFF* tif = XTIFFOpen(test_file, "w");
  ASSERT_NE(tif, nullptr) << "Failed to create test TIFF file";

  // 2. Set basic TIFF fields
  TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, 10);
  TIFFSetField(tif, TIFFTAG_IMAGELENGTH, 10);
  TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
  TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
  TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 10);
  TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

  // 3. Write some dummy data
  unsigned char buf[100] = {0}; // 10x10 image, 1 byte per pixel
  ASSERT_NE(TIFFWriteEncodedStrip(tif, 0, buf, sizeof(buf)), -1)
    << "Failed to write image data";

  // 4. Create GeoTIFF structure
  GTIF* gtif = GTIFNew(tif);
  ASSERT_NE(gtif, nullptr) << "Failed to create GeoTIFF handle";

  // 5. Set a simple GeoTIFF key
  GTIFKeySet(gtif, GTModelTypeGeoKey, TYPE_SHORT, 1, ModelTypeProjected);

  // 6. Write and close
  GTIFWriteKeys(gtif);
  GTIFFree(gtif);
  XTIFFClose(tif);

  // 7. Verify the file was created
  FILE* f = fopen(test_file, "rb");
  ASSERT_NE(f, nullptr) << "Test file was not created";
  fclose(f);

  // 8. Clean up
  ASSERT_EQ(std::remove(test_file), 0) << "Failed to remove test file";
}

// Test reading back a GeoTIFF file
TEST(LibGeoTIFFTest, ReadGeoTIFF)
{
  const char* test_file = "read_test.tif";

  // 1. Create a test file first
  {
    TIFF* tif = XTIFFOpen(test_file, "w");
    ASSERT_NE(tif, nullptr) << "Failed to create test file for reading";

    // Set basic TIFF fields
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, 10);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, 10);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 10);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

    // Write test data
    unsigned char buf[100] = {0};
    for (int i = 0; i < 100; i++)
    {
      buf[i] = static_cast<unsigned char>(i);
    }
    ASSERT_NE(TIFFWriteEncodedStrip(tif, 0, buf, sizeof(buf)), -1);

    // Add GeoTIFF info
    GTIF* gtif = GTIFNew(tif);
    ASSERT_NE(gtif, nullptr) << "Failed to create GeoTIFF handle for writing";

    // Set a test GeoTIFF key
    GTIFKeySet(gtif, GTModelTypeGeoKey, TYPE_SHORT, 1, ModelTypeGeographic);
    GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT, 1, 4326); // WGS84

    GTIFWriteKeys(gtif);
    GTIFFree(gtif);
    XTIFFClose(tif);
  }

  // 2. Read the file back
  TIFF* tif = XTIFFOpen(test_file, "r");
  ASSERT_NE(tif, nullptr) << "Failed to open test file for reading";

  // 3. Verify TIFF fields
  uint32_t width = 0, height = 0;
  uint16_t bits_per_sample = 0, samples_per_pixel = 0;

  ASSERT_TRUE(TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width))
    << "Failed to get image width";
  ASSERT_TRUE(TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height))
    << "Failed to get image height";
  ASSERT_TRUE(TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bits_per_sample))
    << "Failed to get bits per sample";
  ASSERT_TRUE(TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel))
    << "Failed to get samples per pixel";

  EXPECT_EQ(width, 10u) << "Unexpected image width";
  EXPECT_EQ(height, 10u) << "Unexpected image height";
  EXPECT_EQ(bits_per_sample, 8u) << "Unexpected bits per sample";
  EXPECT_EQ(samples_per_pixel, 1u) << "Unexpected samples per pixel";

  // 4. Verify GeoTIFF data
  GTIF* gtif = GTIFNew(tif);
  ASSERT_NE(gtif, nullptr) << "Failed to create GeoTIFF handle for reading";

  short model_type;
  GTIFKeyGet(gtif, GTModelTypeGeoKey, &model_type, 0, 1);
  EXPECT_EQ(model_type, ModelTypeGeographic) << "Unexpected model type";

  short geo_type;
  GTIFKeyGet(gtif, GeographicTypeGeoKey, &geo_type, 0, 1);
  EXPECT_EQ(geo_type, 4326) << "Unexpected geographic type (expected WGS84)";

  // 5. Clean up
  GTIFFree(gtif);
  XTIFFClose(tif);
  std::remove(test_file);
}

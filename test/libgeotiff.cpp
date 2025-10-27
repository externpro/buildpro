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

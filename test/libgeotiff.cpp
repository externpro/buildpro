#include <stdexcept>
#include <string>

#include <boost/filesystem.hpp>

#include <gtest/gtest.h>
#include <libgeotiff/geo_normalize.h>
#include <libgeotiff/geotiffio.h>
#include <libgeotiff/xtiffio.h>

namespace bfs = boost::filesystem;

namespace libgeotiff_test
{

  std::string createTempDirectory()
  {
    // Create a unique temporary directory
    const auto tempDir =
      bfs::temp_directory_path() / "libgeotiff_test_%%%%-%%%%";
    const auto finalPath = bfs::unique_path(tempDir);

    if (!bfs::create_directories(finalPath))
    {
      throw std::runtime_error("Failed to create temporary directory: " +
                               finalPath.string());
    }

    return finalPath.string();
  }

  void removeDirectory(const std::string& path)
  {
    if (bfs::exists(path))
    {
      bfs::remove_all(path);
    }
  }

  class LibGeoTIFFTest : public ::testing::Test
  {
  protected:
    void SetUp() override
    {
      // Create a temporary directory for test files
      test_dir = createTempDirectory();
      test_file = (bfs::path(test_dir) / "test.tif").string();
    }

    void TearDown() override
    {
      // Clean up test files
      if (bfs::exists(test_dir))
      {
        removeDirectory(test_dir);
      }
    }

    std::string test_file;
    std::string test_dir;
  };

} // namespace libgeotiff_test

using namespace libgeotiff_test;

TEST_F(LibGeoTIFFTest, BasicInitialization)
{
  // Test basic TIFF and GeoTIFF initialization
  TIFF* tif = XTIFFOpen(test_file.c_str(), "w");
  ASSERT_NE(tif, nullptr) << "Failed to create TIFF file";

  GTIF* gtif = GTIFNew(tif);
  ASSERT_NE(gtif, nullptr) << "Failed to create GeoTIFF handle";

  // Set some basic TIFF tags
  TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, 100);
  TIFFSetField(tif, TIFFTAG_IMAGELENGTH, 100);
  TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
  TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
  TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);
  TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

  // Set some basic GeoTIFF keys
  GTIFKeySet(gtif, GTModelTypeGeoKey, TYPE_SHORT, 1, ModelTypeProjected);
  GTIFKeySet(
    gtif, ProjectedCSTypeGeoKey, TYPE_SHORT, 1, 32601); // WGS84 / UTM zone 1N

  // Write a simple image
  unsigned char buf[100];
  for (int i = 0; i < 100; ++i)
  {
    buf[i] = static_cast<unsigned char>(i);
    TIFFWriteScanline(tif, buf, i, 0);
  }

  // Write GeoTIFF tags
  GTIFWriteKeys(gtif);
  GTIFFree(gtif);
  XTIFFClose(tif);

  // Now try to read it back
  tif = XTIFFOpen(test_file.c_str(), "r");
  ASSERT_NE(tif, nullptr) << "Failed to open created TIFF file";

  gtif = GTIFNew(tif);
  ASSERT_NE(gtif, nullptr) << "Failed to create GeoTIFF handle for reading";

  // Verify the GeoTIFF keys
  short model;
  GTIFKeyGet(gtif, GTModelTypeGeoKey, &model, 0, 1);
  EXPECT_EQ(model, ModelTypeProjected) << "Unexpected model type";

  short pcs;
  GTIFKeyGet(gtif, ProjectedCSTypeGeoKey, &pcs, 0, 1);
  EXPECT_EQ(pcs, 32601) << "Unexpected PCS value";

  GTIFFree(gtif);
  XTIFFClose(tif);
}

TEST_F(LibGeoTIFFTest, GeoKeyManipulation)
{
  // Test setting and getting various GeoKeys
  TIFF* tif = XTIFFOpen(test_file.c_str(), "w");
  ASSERT_NE(tif, nullptr);

  GTIF* gtif = GTIFNew(tif);
  ASSERT_NE(gtif, nullptr);

  // Set some TIFF fields
  TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, 100);
  TIFFSetField(tif, TIFFTAG_IMAGELENGTH, 100);
  TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
  TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);

  // Set various GeoKeys
  GTIFKeySet(gtif, GTModelTypeGeoKey, TYPE_SHORT, 1, ModelTypeGeographic);
  GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT, 1, 4326);   // WGS84
  GTIFKeySet(gtif, GeogAngularUnitsGeoKey, TYPE_SHORT, 1, 9102); // degrees

  // Write a dummy pixel
  unsigned char pixel = 0;
  TIFFWriteScanline(tif, &pixel, 0, 0);

  // Write and close
  GTIFWriteKeys(gtif);
  GTIFFree(gtif);
  XTIFFClose(tif);

  // Read back and verify
  tif = XTIFFOpen(test_file.c_str(), "r");
  ASSERT_NE(tif, nullptr);

  gtif = GTIFNew(tif);
  ASSERT_NE(gtif, nullptr);

  // Verify GeoKeys
  short model;
  GTIFKeyGet(gtif, GTModelTypeGeoKey, &model, 0, 1);
  EXPECT_EQ(model, ModelTypeGeographic) << "Unexpected model type";

  short geoType;
  GTIFKeyGet(gtif, GeographicTypeGeoKey, &geoType, 0, 1);
  EXPECT_EQ(geoType, 4326) << "Unexpected geographic type";

  short units;
  GTIFKeyGet(gtif, GeogAngularUnitsGeoKey, &units, 0, 1);
  EXPECT_EQ(units, 9102) << "Unexpected angular units";

  GTIFFree(gtif);
  XTIFFClose(tif);
}

TEST_F(LibGeoTIFFTest, GeoPixelScale)
{
  // Test setting and getting pixel scale and tiepoints
  TIFF* tif = XTIFFOpen(test_file.c_str(), "w");
  ASSERT_NE(tif, nullptr);

  GTIF* gtif = GTIFNew(tif);
  ASSERT_NE(gtif, nullptr);

  // Set basic TIFF fields
  TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, 100);
  TIFFSetField(tif, TIFFTAG_IMAGELENGTH, 100);
  TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
  TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);

  // Set model type and projection
  GTIFKeySet(gtif, GTModelTypeGeoKey, TYPE_SHORT, 1, ModelTypeProjected);
  GTIFKeySet(
    gtif, ProjectedCSTypeGeoKey, TYPE_SHORT, 1, 32601); // WGS84 / UTM zone 1N

  // Set pixel scale (30m resolution)
  double pixelScale[3] = {30.0, 30.0, 0.0};
  TIFFSetField(tif, TIFFTAG_GEOPIXELSCALE, 3, pixelScale);

  // Set tiepoint (upper left corner at 500000, 0 in UTM)
  double tiepoints[6] = {0.0, 0.0, 0.0, 500000.0, 1000000.0, 0.0};
  TIFFSetField(tif, TIFFTAG_GEOTIEPOINTS, 6, tiepoints);

  // Write a dummy pixel
  unsigned char pixel = 0;
  TIFFWriteScanline(tif, &pixel, 0, 0);

  // Write and close
  GTIFWriteKeys(gtif);
  GTIFFree(gtif);
  XTIFFClose(tif);

  // Read back and verify
  tif = XTIFFOpen(test_file.c_str(), "r");
  ASSERT_NE(tif, nullptr);

  // Verify pixel scale - use TIFFGetField with proper error checking
  uint16_t nScales = 0;
  double* readScale = nullptr;
  ASSERT_TRUE(TIFFGetField(tif, TIFFTAG_GEOPIXELSCALE, &nScales, &readScale))
    << "Failed to read GEOPIXELSCALE tag";
  ASSERT_GE(nScales, 2) << "Insufficient number of scale values";
  EXPECT_NEAR(readScale[0], 30.0, 1e-6) << "X scale mismatch";
  EXPECT_NEAR(readScale[1], 30.0, 1e-6) << "Y scale mismatch";

  // Verify tiepoints
  uint16_t nTiepoints = 0;
  double* readTiepoints = nullptr;
  ASSERT_TRUE(
    TIFFGetField(tif, TIFFTAG_GEOTIEPOINTS, &nTiepoints, &readTiepoints))
    << "Failed to read GEOTIEPOINTS tag";
  ASSERT_GE(nTiepoints, 6) << "Not enough tiepoints";
  EXPECT_NEAR(readTiepoints[3], 500000.0, 1e-6) << "X coordinate mismatch";
  EXPECT_NEAR(readTiepoints[4], 1000000.0, 1e-6) << "Y coordinate mismatch";

  XTIFFClose(tif);
}

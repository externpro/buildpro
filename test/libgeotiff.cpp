#include <chrono>
#include <ctime>
#include <future>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <boost/filesystem.hpp>

#include <gtest/gtest.h>
#include <libgeotiff/geo_normalize.h>
#include <libgeotiff/geotiffio.h>
#include <libgeotiff/xtiffio.h>

namespace bfs = boost::filesystem;

namespace libgeotiff_test
{

  // Simple RAII wrapper for TIFF files
  class TiffFile
  {
  public:
    TiffFile(const std::string& path, const char* mode)
      : tif_(XTIFFOpen(path.c_str(), mode))
    {
    }

    ~TiffFile()
    {
      if (tif_)
        XTIFFClose(tif_);
    }

    operator bool() const { return tif_ != nullptr; }
    TIFF* get() const { return tif_; }

    // Disable copying
    TiffFile(const TiffFile&) = delete;
    TiffFile& operator=(const TiffFile&) = delete;

  private:
    TIFF* tif_;
  };

  // RAII wrapper for GTIF
  class GtifHandle
  {
  public:
    GtifHandle(TIFF* tif) : gtif_(GTIFNew(tif)) { }
    ~GtifHandle()
    {
      if (gtif_)
        GTIFFree(gtif_);
    }

    operator bool() const { return gtif_ != nullptr; }
    GTIF* get() const { return gtif_; }

    // Disable copying
    GtifHandle(const GtifHandle&) = delete;
    GtifHandle& operator=(const GtifHandle&) = delete;

  private:
    GTIF* gtif_;
  };

  std::string createTempDirectory()
  {
    // Use a fixed directory with a timestamp to avoid cleanup issues
    const auto finalPath = bfs::temp_directory_path() / "gt_test_libgeotiff";

    try
    {
      // Clean up any previous test run
      if (bfs::exists(finalPath))
      {
        bfs::remove_all(finalPath);
      }

      if (!bfs::create_directories(finalPath))
      {
        throw std::runtime_error("Failed to create directory: " +
                                 finalPath.string());
      }

      return finalPath.string();
    }
    catch (const std::exception& e)
    {
      // If we can't create in temp dir, try current directory
      const auto fallback = bfs::current_path() / "gt_test_temp";
      if (!bfs::exists(fallback))
      {
        bfs::create_directories(fallback);
      }
      return fallback.string();
    }
  }

  void removeDirectory(const std::string& path)
  {
    if (!bfs::exists(path))
    {
      return;
    }

    // Try multiple times to handle Windows file locking
    int attempts = 3;
    while (attempts-- > 0)
    {
      try
      {
        bfs::remove_all(path);
        return;
      }
      catch (const std::exception& e)
      {
        if (attempts == 0)
        {
          std::cerr << "Warning: Failed to remove directory " << path << ": "
                    << e.what() << std::endl;
          return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
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
      // Clean up test files with retry logic for Windows
      int attempts = 3;
      while (attempts-- > 0)
      {
        try
        {
          if (bfs::exists(test_dir))
          {
            removeDirectory(test_dir);
          }
          break;
        }
        catch (const std::exception& e)
        {
          if (attempts == 0)
          {
            std::cerr << "Warning: Failed to clean up test directory: "
                      << e.what() << std::endl;
          }
          else
          {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
          }
        }
      }
    }

    std::string test_file;
    std::string test_dir;
  };

} // namespace libgeotiff_test

using namespace libgeotiff_test;

// Helper function to run a test with timeout
#define RUN_TEST_WITH_TIMEOUT(ms, test_func)                                   \
  do                                                                           \
  {                                                                            \
    std::promise<bool> completed;                                              \
    auto future = completed.get_future();                                      \
    std::thread(                                                               \
      [&]()                                                                    \
      {                                                                        \
        try                                                                    \
        {                                                                      \
          test_func();                                                         \
          completed.set_value(true);                                           \
        }                                                                      \
        catch (...)                                                            \
        {                                                                      \
          completed.set_exception(std::current_exception());                   \
        }                                                                      \
      })                                                                       \
      .detach();                                                               \
    if (future.wait_for(std::chrono::milliseconds(ms)) ==                      \
        std::future_status::timeout)                                           \
    {                                                                          \
      FAIL() << "Test timed out after " << ms << "ms";                         \
    }                                                                          \
    future.get();                                                              \
  } while (0)

TEST_F(LibGeoTIFFTest, BasicInitialization)
{
  RUN_TEST_WITH_TIMEOUT(5000, [&]() {  // 5 second timeout
    // Test basic TIFF and GeoTIFF initialization
    TiffFile tif(test_file, "w");
    ASSERT_TRUE(tif) << "Failed to create TIFF file";

    GtifHandle gtif(tif.get());
    ASSERT_TRUE(gtif) << "Failed to create GeoTIFF handle";

    // Set some basic TIFF tags
    TIFFSetField(tif.get(), TIFFTAG_IMAGEWIDTH, 100);
    TIFFSetField(tif.get(), TIFFTAG_IMAGELENGTH, 100);
    TIFFSetField(tif.get(), TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(tif.get(), TIFFTAG_SAMPLESPERPIXEL, 1);
    TIFFSetField(tif.get(), TIFFTAG_ROWSPERSTRIP, 1);
    TIFFSetField(tif.get(), TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

    // Set some basic GeoTIFF keys
    GTIFKeySet(gtif.get(), GTModelTypeGeoKey, TYPE_SHORT, 1, ModelTypeProjected);
    GTIFKeySet(gtif.get(), ProjectedCSTypeGeoKey, TYPE_SHORT, 1, 32601); // WGS84 / UTM zone 1N

    // Write a simple image
    std::vector<unsigned char> buf(100);
    for (int i = 0; i < 100; ++i) {
      buf[i] = static_cast<unsigned char>(i);
      if (TIFFWriteScanline(tif.get(), buf.data(), i, 0) != 1) {
        throw std::runtime_error("Failed to write scanline " + std::to_string(i));
      }
    }

    // Write and close
    GTIFWriteKeys(gtif.get());
  }); // End of timeout wrapper

  // Now try to read it back in a separate operation
  RUN_TEST_WITH_TIMEOUT(
    5000,
    [&]()
    {
      TiffFile tif(test_file, "r");
      ASSERT_TRUE(tif) << "Failed to open created TIFF file";

      GtifHandle gtif(tif.get());
      ASSERT_TRUE(gtif) << "Failed to create GeoTIFF handle for reading";

      // Verify the GeoTIFF keys
      short model;
      if (!GTIFKeyGet(gtif.get(), GTModelTypeGeoKey, &model, 0, 1))
      {
        throw std::runtime_error("Failed to get GTModelTypeGeoKey");
      }
      EXPECT_EQ(model, ModelTypeProjected) << "Unexpected model type";

      short pcs;
      if (!GTIFKeyGet(gtif.get(), ProjectedCSTypeGeoKey, &pcs, 0, 1))
      {
        throw std::runtime_error("Failed to get ProjectedCSTypeGeoKey");
      }
      EXPECT_EQ(pcs, 32601) << "Unexpected PCS value";
    });
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

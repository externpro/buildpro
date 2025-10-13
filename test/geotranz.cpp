#include <cstdlib>
#include <iostream>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <geotranz/engine.h>
#include <geotranz/geocent.h>
#include <geotranz/mgrs.h>
#include <geotranz/utm.h>
#include <gtest/gtest.h>

// Helper class to manage GEOTRANZ engine initialization/cleanup
class GEOTRANZTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
#ifdef GEOTRANS_DATA
    // Set the GEOTRANS_DATA environment variable
#ifdef _WIN32
    if (_putenv_s("GEOTRANS_DATA", GEOTRANS_DATA) != 0)
    {
      FAIL() << "Failed to set GEOTRANS_DATA environment variable";
    }
#else
    if (setenv("GEOTRANS_DATA", GEOTRANS_DATA, 1) != 0)
    {
      FAIL() << "Failed to set GEOTRANS_DATA environment variable";
    }
#endif

    // Initialize the GEOTRANZ engine
    long error = Initialize_Engine();

    if (error != 0)
    {
      std::string error_msg = "GEOTRANZ initialization failed with error: 0x";
      error_msg += std::to_string(error);
      if (error & ENGINE_ELLIPSOID_ERROR)
        error_msg += "\n- Ellipsoid module error (check ellips.dat)";
      if (error & ENGINE_DATUM_ERROR)
        error_msg += "\n- Datum module error (check 3_param.dat, 7_param.dat, "
                     "geod_datum.csv, geod_trf.csv)";
      if (error & ENGINE_GEOID_ERROR)
        error_msg += "\n- Geoid module error (check egm84.grd, egm96.grd)";
      FAIL() << error_msg;
    }
#else
    FAIL() << "GEOTRANS_DATA not defined at compile time";
#endif

    // Set UTM parameters (WGS84 ellipsoid)
    double a = 6378137.0;           // semi-major axis in meters
    double f = 1.0 / 298.257223563; // flattening
    long override = 0;              // no zone override

    error = Set_UTM_Parameters(a, f, override);
    if (error != 0)
    {
      std::cerr << "Set_UTM_Parameters failed with error: " << error
                << std::endl;
    }
    ASSERT_EQ(error, 0) << "Failed to set UTM parameters";
  }

  // Helper function to convert degrees to radians
  double DegreesToRadians(double degrees) const
  {
    return degrees * M_PI / 180.0;
  }

  // Helper function to convert radians to degrees
  double RadiansToDegrees(double radians) const
  {
    return radians * 180.0 / M_PI;
  }
};

// Test basic UTM conversion
TEST_F(GEOTRANZTest, UTMConversion)
{
  // Set UTM parameters (WGS84 ellipsoid)
  double a = 6378137.0;           // semi-major axis in meters
  double f = 1.0 / 298.257223563; // flattening
  long override = 0;              // no zone override

  long error = Set_UTM_Parameters(a, f, override);
  ASSERT_EQ(error, UTM_NO_ERROR) << "Failed to set UTM parameters";

  // Test coordinates (Washington DC: 38.8977° N, 77.0365° W)
  double lat = DegreesToRadians(38.8977);
  double lon = DegreesToRadians(-77.0365);

  // Convert geodetic to UTM
  long zone;
  char hemisphere;
  double easting, northing;

  error =
    Convert_Geodetic_To_UTM(lat, lon, &zone, &hemisphere, &easting, &northing);
  ASSERT_EQ(error, UTM_NO_ERROR) << "Failed to convert geodetic to UTM";

  // Verify UTM zone and hemisphere
  EXPECT_EQ(zone, 18);
  EXPECT_EQ(hemisphere, 'N');

  // Convert back to geodetic
  double lat2, lon2;
  error =
    Convert_UTM_To_Geodetic(zone, hemisphere, easting, northing, &lat2, &lon2);
  ASSERT_EQ(error, UTM_NO_ERROR) << "Failed to convert UTM to geodetic";

  // Verify the round-trip conversion (allowing for small numerical differences)
  EXPECT_NEAR(RadiansToDegrees(lat), RadiansToDegrees(lat2), 0.000001);
  EXPECT_NEAR(RadiansToDegrees(lon), RadiansToDegrees(lon2), 0.000001);
}

// Test MGRS conversion
TEST_F(GEOTRANZTest, MGRSConversion)
{
  // Set MGRS parameters (WGS84 ellipsoid)
  double a = 6378137.0;           // semi-major axis in meters
  double f = 1.0 / 298.257223563; // flattening
  char ellipsoid_code[3] = "WE";  // WGS84 ellipsoid code

  long error = Set_MGRS_Parameters(a, f, ellipsoid_code);
  ASSERT_EQ(error, MGRS_NO_ERROR) << "Failed to set MGRS parameters";

  // Test coordinates (Mount Everest: 27.9881° N, 86.9250° E)
  double lat = DegreesToRadians(27.9881);
  double lon = DegreesToRadians(86.9250);

  // Convert geodetic to MGRS with precision 5 (1m)
  char mgrs[21];
  long precision = 5;

  error = Convert_Geodetic_To_MGRS(lat, lon, precision, mgrs);
  ASSERT_EQ(error, MGRS_NO_ERROR) << "Failed to convert geodetic to MGRS";

  // Convert MGRS back to geodetic
  double lat2, lon2;
  error = Convert_MGRS_To_Geodetic(mgrs, &lat2, &lon2);
  ASSERT_EQ(error, MGRS_NO_ERROR) << "Failed to convert MGRS to geodetic";

  // Verify the round-trip conversion (allowing for small numerical differences
  // due to precision)
  EXPECT_NEAR(RadiansToDegrees(lat), RadiansToDegrees(lat2), 0.00001);
  EXPECT_NEAR(RadiansToDegrees(lon), RadiansToDegrees(lon2), 0.00001);
}

// Test UTM to MGRS conversion
TEST_F(GEOTRANZTest, UTMToMGRSConversion)
{
  // Set UTM and MGRS parameters (WGS84 ellipsoid)
  double a = 6378137.0;           // semi-major axis in meters
  double f = 1.0 / 298.257223563; // flattening
  long override = 0;              // no zone override
  char ellipsoid_code[3] = "WE";  // WGS84 ellipsoid code

  Set_UTM_Parameters(a, f, override);
  Set_MGRS_Parameters(a, f, ellipsoid_code);

  // Test coordinates (Sydney, Australia: 33.8688° S, 151.2093° E)
  double lat = DegreesToRadians(-33.8688);
  double lon = DegreesToRadians(151.2093);

  // Convert geodetic to UTM
  long zone;
  char hemisphere;
  double easting, northing;

  long error =
    Convert_Geodetic_To_UTM(lat, lon, &zone, &hemisphere, &easting, &northing);
  ASSERT_EQ(error, UTM_NO_ERROR) << "Failed to convert geodetic to UTM";

  // Convert UTM to MGRS
  char mgrs[21];
  long precision = 5;

  error =
    Convert_UTM_To_MGRS(zone, hemisphere, easting, northing, precision, mgrs);
  ASSERT_EQ(error, MGRS_NO_ERROR) << "Failed to convert UTM to MGRS";

  // Convert MGRS back to UTM
  long zone2;
  char hemisphere2;
  double easting2, northing2;

  error =
    Convert_MGRS_To_UTM(mgrs, &zone2, &hemisphere2, &easting2, &northing2);
  ASSERT_EQ(error, MGRS_NO_ERROR) << "Failed to convert MGRS to UTM";

  // Verify the round-trip conversion
  EXPECT_EQ(zone, zone2);
  EXPECT_EQ(hemisphere, hemisphere2);
  // Increased tolerance for UTM to MGRS round-trip conversion
  EXPECT_NEAR(easting, easting2, 1.0)
    << "Easting values differ by " << std::abs(easting - easting2);
  EXPECT_NEAR(northing, northing2, 1.0)
    << "Northing values differ by " << std::abs(northing - northing2);
}

// Test Geocentric conversion
TEST_F(GEOTRANZTest, GeocentricConversion)
{
  // Set Geocentric parameters (WGS84 ellipsoid)
  double a = 6378137.0;           // semi-major axis in meters
  double f = 1.0 / 298.257223563; // flattening

  long error = Set_Geocentric_Parameters(a, f);
  ASSERT_EQ(error, GEOCENT_NO_ERROR) << "Failed to set Geocentric parameters";

  // Test coordinates (Tokyo: 35.6762° N, 139.6503° E, 0m height)
  double lat = DegreesToRadians(35.6762);
  double lon = DegreesToRadians(139.6503);
  double height = 0.0;

  // Convert geodetic to geocentric
  double x, y, z;

  error = Convert_Geodetic_To_Geocentric(lat, lon, height, &x, &y, &z);
  ASSERT_EQ(error, GEOCENT_NO_ERROR)
    << "Failed to convert geodetic to geocentric";

  // Convert geocentric back to geodetic
  double lat2, lon2, height2;
  Convert_Geocentric_To_Geodetic(x, y, z, &lat2, &lon2, &height2);
  // Note: Convert_Geocentric_To_Geodetic is void and doesn't return an error
  // code

  // Verify the round-trip conversion
  EXPECT_NEAR(RadiansToDegrees(lat), RadiansToDegrees(lat2), 0.000001);
  EXPECT_NEAR(RadiansToDegrees(lon), RadiansToDegrees(lon2), 0.000001);
  // Increased tolerance for height due to geodetic to geocentric conversion
  // precision
  EXPECT_NEAR(height, height2, 0.01)
    << "Height values differ by " << std::abs(height - height2);
}

// Test coordinate system conversion using the Engine interface
TEST_F(GEOTRANZTest, CoordinateSystemConversion)
{
  // Set coordinate systems for input and output
  long error = Set_Coordinate_System(Interactive, Input, Geodetic);
  ASSERT_EQ(error, ENGINE_NO_ERROR) << "Failed to set input coordinate system";

  error = Set_Coordinate_System(Interactive, Output, UTM);
  ASSERT_EQ(error, ENGINE_NO_ERROR) << "Failed to set output coordinate system";

  // Verify the coordinate systems were set correctly
  Coordinate_Type input_system, output_system;

  error = Get_Coordinate_System(Interactive, Input, &input_system);
  ASSERT_EQ(error, ENGINE_NO_ERROR) << "Failed to get input coordinate system";
  EXPECT_EQ(input_system, Geodetic);

  error = Get_Coordinate_System(Interactive, Output, &output_system);
  ASSERT_EQ(error, ENGINE_NO_ERROR) << "Failed to get output coordinate system";
  EXPECT_EQ(output_system, UTM);
}

// Test error handling for invalid inputs
TEST_F(GEOTRANZTest, ErrorHandling)
{
  // Set UTM parameters
  double a = 6378137.0;
  double f = 1.0 / 298.257223563;
  long override = 0;

  Set_UTM_Parameters(a, f, override);

  // Test invalid latitude (outside -80.5 to 84.5 degrees for UTM)
  double lat = DegreesToRadians(-85.0);
  double lon = DegreesToRadians(0.0);

  long zone;
  char hemisphere;
  double easting, northing;

  long error =
    Convert_Geodetic_To_UTM(lat, lon, &zone, &hemisphere, &easting, &northing);
  EXPECT_NE(error, UTM_NO_ERROR);
  EXPECT_TRUE((error & UTM_LAT_ERROR) != 0)
    << "UTM_LAT_ERROR not set for invalid latitude";

  // Test invalid longitude
  lat = DegreesToRadians(0.0);
  lon = DegreesToRadians(400.0);

  error =
    Convert_Geodetic_To_UTM(lat, lon, &zone, &hemisphere, &easting, &northing);
  EXPECT_NE(error, UTM_NO_ERROR);
  EXPECT_TRUE((error & UTM_LON_ERROR) != 0)
    << "UTM_LON_ERROR not set for invalid longitude";
}

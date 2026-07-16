#include <memory>
#include <stdexcept>
#include <string>

#include <geos_c.h>
#include <gtest/gtest.h>

class GEOSHandle
{
public:
  GEOSHandle() : handle(GEOS_init_r()) { }
  ~GEOSHandle()
  {
    if (handle)
      GEOS_finish_r(handle);
  }
  GEOSContextHandle_t get() const { return handle; }

private:
  GEOSContextHandle_t handle;
};

class GEOSTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Initialize GEOS context
    geosHandle = std::make_unique<GEOSHandle>();
    handle = geosHandle->get();
    reader = GEOSWKTReader_create_r(handle);
    writer = GEOSWKTWriter_create_r(handle);
    GEOSWKTWriter_setTrim_r(handle, writer, 1);
    GEOSWKTWriter_setRoundingPrecision_r(handle, writer, 3);
  }

  void TearDown() override
  {
    if (writer)
      GEOSWKTWriter_destroy_r(handle, writer);
    if (reader)
      GEOSWKTReader_destroy_r(handle, reader);
  }

  GEOSGeometry* readWKT(const std::string& wkt)
  {
    return GEOSWKTReader_read_r(handle, reader, wkt.c_str());
  }

  std::string writeWKT(GEOSGeometry* geom)
  {
    char* wkt = GEOSWKTWriter_write_r(handle, writer, geom);
    if (!wkt)
      throw std::runtime_error("Failed to write WKT");
    std::string result(wkt);
    GEOSFree_r(handle, wkt);
    return result;
  }

  std::unique_ptr<GEOSHandle> geosHandle;
  GEOSContextHandle_t handle;
  GEOSWKTReader* reader = nullptr;
  GEOSWKTWriter* writer = nullptr;
};

TEST_F(GEOSTest, CreatePoint)
{
  // Create a point at (1, 2)
  GEOSCoordSequence* coord = GEOSCoordSeq_create_r(handle, 1, 2);
  ASSERT_NE(coord, nullptr);

  GEOSCoordSeq_setX_r(handle, coord, 0, 1.0);
  GEOSCoordSeq_setY_r(handle, coord, 0, 2.0);

  GEOSGeometry* point = GEOSGeom_createPoint_r(handle, coord);
  ASSERT_NE(point, nullptr);

  // Verify point properties
  EXPECT_EQ(GEOSGeomTypeId_r(handle, point), GEOS_POINT);

  // Clean up
  GEOSGeom_destroy_r(handle, point);
}

TEST_F(GEOSTest, WKTParsing)
{
  // Test WKT parsing and writing
  const char* wkt =
    "POLYGON((0 0, 0 10, 10 10, 10 0, 0 0), (2 2, 2 8, 8 8, 8 2, 2 2))";

  GEOSGeometry* geom = readWKT(wkt);
  ASSERT_NE(geom, nullptr);

  // Convert back to WKT and verify the geometry type
  std::string result = writeWKT(geom);
  EXPECT_NE(result.find("POLYGON"), std::string::npos);

  // Clean up
  GEOSGeom_destroy_r(handle, geom);
}

TEST_F(GEOSTest, SpatialRelations)
{
  // Create two polygons that intersect
  GEOSGeometry* poly1 = readWKT("POLYGON((0 0, 0 5, 5 5, 5 0, 0 0))");
  GEOSGeometry* poly2 = readWKT("POLYGON((3 3, 3 8, 8 8, 8 3, 3 3))");

  ASSERT_NE(poly1, nullptr);
  ASSERT_NE(poly2, nullptr);

  // Test spatial relationships
  char intersects = GEOSIntersects_r(handle, poly1, poly2);
  char contains = GEOSContains_r(handle, poly1, poly2);

  EXPECT_EQ(intersects, 1);
  EXPECT_NE(contains, 1);

  // Calculate intersection
  GEOSGeometry* intersection = GEOSIntersection_r(handle, poly1, poly2);
  EXPECT_NE(intersection, nullptr);
  EXPECT_EQ(GEOSGeomTypeId_r(handle, intersection), GEOS_POLYGON);

  // Clean up
  GEOSGeom_destroy_r(handle, intersection);
  GEOSGeom_destroy_r(handle, poly1);
  GEOSGeom_destroy_r(handle, poly2);
}

TEST_F(GEOSTest, BufferOperation)
{
  // Create a point
  GEOSGeometry* point = readWKT("POINT(0 0)");
  ASSERT_NE(point, nullptr);

  // Create a buffer around the point
  GEOSGeometry* buffer = GEOSBuffer_r(handle, point, 5.0, 8);
  ASSERT_NE(buffer, nullptr);

  // Verify the buffer is a polygon and has expected area
  EXPECT_EQ(GEOSGeomTypeId_r(handle, buffer), GEOS_POLYGON);

  double area = 0.0;
  GEOSArea_r(handle, buffer, &area);
  EXPECT_GT(area, 0);

  // The area should be approximately pi*r² (allowing for some approximation
  // error)
  EXPECT_NEAR(area, 78.5, 5.0);

  // Clean up
  GEOSGeom_destroy_r(handle, buffer);
  GEOSGeom_destroy_r(handle, point);
}

TEST_F(GEOSTest, DistanceCalculation)
{
  // Create two points
  GEOSGeometry* point1 = readWKT("POINT(0 0)");
  GEOSGeometry* point2 = readWKT("POINT(3 4)");

  ASSERT_NE(point1, nullptr);
  ASSERT_NE(point2, nullptr);

  // Calculate distance (should be 5 for 3-4-5 triangle)
  double distance = 0.0;
  int result = GEOSDistance_r(handle, point1, point2, &distance);

  EXPECT_EQ(result, 1); // GEOSDistance_r returns 1 on success
  EXPECT_DOUBLE_EQ(distance, 5.0);

  // Clean up
  GEOSGeom_destroy_r(handle, point1);
  GEOSGeom_destroy_r(handle, point2);
}

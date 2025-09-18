set(SPATIALITE_TOOL "@SPATIALITE_TOOL@")
# Create a temporary directory for the test
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_temp")
set(TEST_DB "${CMAKE_CURRENT_BINARY_DIR}/test_temp/test_country_boundaries.sqlite")
set(TEST_SQL "psql_country_boundaries_mod.sql")
set(TEST_SQL_URL "https://github.com/externpro/spatialite-tools/releases/download/v5.1.0.2/${TEST_SQL}")
# Make sure we start with a clean state
if(EXISTS "${TEST_DB}")
  file(REMOVE "${TEST_DB}")
endif()
# Download the SQL file if it doesn't exist
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${TEST_SQL}")
  message(STATUS "Downloading test data...")
  file(DOWNLOAD "${TEST_SQL_URL}" "${CMAKE_CURRENT_BINARY_DIR}/${TEST_SQL}"
    STATUS download_status
    SHOW_PROGRESS
    )
  list(GET download_status 0 status_code)
  if(NOT status_code EQUAL 0)
    message(FATAL_ERROR "Failed to download test data")
  endif()
endif()
# Create a new SpatiaLite database and import the data
execute_process(
  COMMAND ${SPATIALITE_TOOL} "${TEST_DB}" ".read ${CMAKE_CURRENT_BINARY_DIR}/${TEST_SQL}"
  RESULT_VARIABLE result
  OUTPUT_VARIABLE output
  ERROR_VARIABLE error
  )
if(NOT result EQUAL 0)
  message(FATAL_ERROR "Failed to create test database: ${error}\nOutput: ${output}")
endif()
# Test the spatial query
set(TEST_QUERY "SELECT COUNT(*) FROM country_boundaries WHERE Intersects(GeomFromText('POLYGON((0 0, 0 10, 10 10, 10 0, 0 0))'), country_boundaries.geom);")
execute_process(
  COMMAND ${SPATIALITE_TOOL} "${TEST_DB}" "${TEST_QUERY}"
  RESULT_VARIABLE result
  OUTPUT_VARIABLE output
  ERROR_VARIABLE error
  )
if(NOT result EQUAL 0)
  message(FATAL_ERROR "Spatial query failed: ${error}\nOutput: ${output}")
endif()
# Clean up the test database
file(REMOVE "${TEST_DB}")
# Verify the result (expecting 8 countries in the specified region)
if(NOT output MATCHES "8")
  message(FATAL_ERROR "Unexpected result from spatial query. Expected 8 countries, got: ${output}")
endif()
message(STATUS "Spatial query test passed successfully")

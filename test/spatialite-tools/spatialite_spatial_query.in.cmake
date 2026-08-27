set(SPATIALITE_TOOL "@SPATIALITE_TOOL@")
set(TEST_TEMP_DIR "${CMAKE_CURRENT_BINARY_DIR}/test_temp")
set(TEST_SQL "psql_country_boundaries_mod.sql")
set(TEST_SQL_URL "https://github.com/externpro/spatialite-tools/releases/download/v5.1.0.2/${TEST_SQL}")

# Clean up any previous test artifacts
if(EXISTS "${TEST_TEMP_DIR}")
  file(REMOVE_RECURSE "${TEST_TEMP_DIR}")
endif()
file(MAKE_DIRECTORY "${TEST_TEMP_DIR}")

# Always delete cached SQL file to ensure fresh download (prevents Debug/Release conflicts)
file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/${TEST_SQL}")

# Download the SQL file with 30-second timeout (robust for CI systems)
message(STATUS "Downloading test data...")
file(DOWNLOAD "${TEST_SQL_URL}" "${CMAKE_CURRENT_BINARY_DIR}/${TEST_SQL}"
  STATUS download_status
  SHOW_PROGRESS
  TIMEOUT 30
  )
list(GET download_status 0 status_code)

if(NOT status_code EQUAL 0)
  message(STATUS "Failed to download test data - skipping test (offline mode)")
  file(REMOVE_RECURSE "${TEST_TEMP_DIR}")
  return()
endif()

# Use a unique database name to avoid conflicts
string(RANDOM LENGTH 8 RANDOM_SUFFIX)
set(TEST_DB "${TEST_TEMP_DIR}/test_country_boundaries_${RANDOM_SUFFIX}.sqlite")

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

# Verify the result (expecting 8 countries in the specified region)
if(NOT output MATCHES "8")
  message(FATAL_ERROR "Unexpected result from spatial query. Expected 8 countries, got: ${output}")
endif()

message(STATUS "Spatial query test passed successfully")

# Clean up
file(REMOVE_RECURSE "${TEST_TEMP_DIR}")
file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/${TEST_SQL}")

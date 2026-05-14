xpGetPkgVar(spatialite-tools VER)
xpFindPkg(PKGS libspatialite SQLite3)
xpGetPkgVar(libspatialite VER)
xpGetPkgVar(SQLite3 VER)
set(tools
  spatialite
  spatialite_convert
  spatialite_network
  spatialite_tool
  shp_doctor
  exif_loader
  )
foreach(tool ${tools})
  set(tool_EXE $<TARGET_FILE:xpro::${tool}>)
  set(tool_CMAKE ${CMAKE_CURRENT_BINARY_DIR}/${tool}.cmake)
  configure_file(tool.in.cmake ${tool_CMAKE} @ONLY)
  file(GENERATE OUTPUT ${tool_CMAKE} INPUT ${tool_CMAKE})
  add_test(NAME check_${tool}_version COMMAND ${CMAKE_COMMAND} -P ${tool_CMAKE})
endforeach()
# spatialite_spatial_query test
find_program(SPATIALITE_TOOL spatialite
  PATHS "${spatialite-tools_DIR}/../../bin"
  NO_DEFAULT_PATH
  )
if(NOT SPATIALITE_TOOL)
  message(FATAL_ERROR "spatialite tool not found")
endif()
set(testName spatialite_spatial_query)
configure_file(${testName}.in.cmake ${testName}.cmake @ONLY)
add_test(NAME ${testName} COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/${testName}.cmake")
set_tests_properties(${testName} PROPERTIES
  ENVIRONMENT "SPATIALITE_TOOL=${SPATIALITE_TOOL}"
# LABELS "spatialite;spatial_query;integration"
  )

set(pro nvJPEG2000)
xpGetPkgVar(${pro} VER)
# parse the version string into major, minor, and patch
string(REGEX MATCH "^(xpv|v)([0-9]+)\\.([0-9]+)\\.([0-9]+)\\.([0-9]+)" ${pro}_VER ${${pro}_VER})
set(VERSION_MAJOR ${CMAKE_MATCH_2})
set(VERSION_MINOR ${CMAKE_MATCH_3})
set(VERSION_PATCH ${CMAKE_MATCH_4})
configure_file(nvjpeg2000.in.cpp nvjpeg2000.cpp @ONLY)
# nvjpeg2000 is not available on macOS
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  add_executable(${pro}_test ${CMAKE_CURRENT_BINARY_DIR}/nvjpeg2000.cpp)
  target_link_libraries(${pro}_test PRIVATE ${NVJPEG2000_LIBRARIES} xpro::gtest_main)
  add_test(NAME ${pro}_test COMMAND ${pro}_test)
endif()

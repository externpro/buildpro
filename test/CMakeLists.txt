list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
set(pros
  googletest
  apr
  argon2
  azmq
  boost
  bzip2
  ceres-solver
  clang-format
  eigen
  fecpp
  flatbuffers
  fmt
  geos
  geotranz
  hdf5
  jasper
  jxrlib
  libgeotiff
  libiconv
  librttopo
  libsodium
  libspatialite
  libstrophe
  libzmq
  nasm
  nlohmann_json
  nodeng
  nodexp
  node-addon-api
  nvjpeg2000
  openh264
  patch
  rapidjson
  spatialite-tools
  spdlog
  sqlite3
  wirehair
  wxinclude
  wxtetris
  wxwidgets
  wxx
  yasm
  zlib
  )
list(APPEND pros
  # exdlpro projects on all platforms
  activemqcpp
  cares
  cppzmq
  curl
  expat
  glew
  jpegxp
  libgit2
  libssh2
  lua
  openssl
  protobuf
  rapidxml
  shapelib
  zmqpp
  )
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  list(APPEND pros
    # exdlpro projects not on macOS
    ffmpeg
    )
endif()
find_program(XVFB_RUN_EXEC xvfb-run)
if(XVFB_RUN_EXEC)
  set(xvfb_cmd ${XVFB_RUN_EXEC})
endif()
# NOTE: set ${pro}_libs if ${PRJ}_LIBRARIES isn't sufficient
set(googletest_libs xpro::gtest_main)
set(boost_libs Boost::filesystem Boost::program_options Boost::regex)
set(boost_compile_definitions "BOOST_NO_CXX98_FUNCTION_BASE")
set(fmt_libs xpro::fmt)
xpGetPkgVar(geotranz DATA_DIR) # sets GEOTRANZ_DATA_DIR
set(geotranz_compile_definitions "GEOTRANS_DATA=\"${GEOTRANZ_DATA_DIR}\"")
set(hdf5_libs xpro::hdf5_cpp-static)
set(libgeotiff_libs xpro::libgeotiff Boost::filesystem)
set(libgit2_libs xpro::git2 Boost::filesystem)
set(libiconv_libs xpro::libiconv)
set(libzmq_compile_features cxx_std_17)
set(nlohmann_json_compile_features cxx_std_17)
set(shapelib_libs xpro::shape Boost::filesystem)
set(spdlog_libs xpro::spdlog)
set(spdlog_compile_features cxx_std_17)
set(wxwidgets_libs wx::base wx::core)
set(wxx_xvfb ${xvfb_cmd})
foreach(pro ${pros})
  find_package(${pro})
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${pro}.cpp)
    add_executable(${pro}_test ${pro}.cpp)
    list(APPEND all_srcs ${pro}.cpp)
    string(TOUPPER ${pro} PRJ)
    if(DEFINED ${pro}_libs)
      target_link_libraries(${pro}_test PRIVATE ${${pro}_libs} xpro::gtest_main)
      list(APPEND all_libs ${${pro}_libs})
    elseif(DEFINED ${PRJ}_LIBRARIES)
      target_link_libraries(${pro}_test PRIVATE ${${PRJ}_LIBRARIES} xpro::gtest_main)
      list(APPEND all_libs ${${PRJ}_LIBRARIES})
    endif()
    if(DEFINED ${pro}_compile_features)
      target_compile_features(${pro}_test PRIVATE ${${pro}_compile_features})
      list(APPEND all_compile_features ${${pro}_compile_features})
    endif()
    if(DEFINED ${pro}_compile_definitions)
      target_compile_definitions(${pro}_test PRIVATE ${${pro}_compile_definitions})
      list(APPEND all_compile_definitions ${${pro}_compile_definitions})
    endif()
    add_test(NAME ${pro}_test COMMAND ${${pro}_xvfb} $<TARGET_FILE:${pro}_test>)
  endif()
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${pro}.cmake)
    xpGetPkgVar(${pro} VER)
    xpGetPkgVar(${pro} EXE)
    if(TARGET ${${pro}_EXE})
      set(${pro}_EXE_PATH $<TARGET_FILE:${${pro}_EXE}>)
    elseif(pro STREQUAL "protobuf")
      xpGetPkgVar(${pro} PROTOC_EXECUTABLE)
      if(TARGET ${${pro}_PROTOC_EXECUTABLE})
        set(${pro}_EXE_PATH $<TARGET_FILE:${${pro}_PROTOC_EXECUTABLE}>)
      endif()
    else()
      set(${pro}_EXE_PATH ${${pro}_EXE})
    endif()
    add_test(NAME ${pro}_exe_test COMMAND ${CMAKE_COMMAND}
      -D${pro}_VER='${${pro}_VER}' -D${pro}_EXE='${${pro}_EXE_PATH}'
      -P ${CMAKE_CURRENT_SOURCE_DIR}/${pro}.cmake
      )
  endif()
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${pro}/CMakeLists.txt)
    add_subdirectory(${pro})
  endif()
endforeach()
list(REMOVE_DUPLICATES all_libs)
list(REMOVE_DUPLICATES all_compile_features)
list(REMOVE_DUPLICATES all_compile_definitions)
# TRICKY: libsodium includes argon2, so we need to remove it
# from the lists to avoid duplicate symbols at link-time
list(REMOVE_ITEM all_srcs argon2.cpp)
list(REMOVE_ITEM all_libs ${ARGON2_LIBRARIES})
add_executable(toda ${all_srcs})
target_link_libraries(toda PRIVATE ${all_libs})
target_compile_features(toda PRIVATE ${all_compile_features})
target_compile_definitions(toda PRIVATE ${all_compile_definitions})
xpSourceListAppend()

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
  patch
  rapidjson
  spatialite-tools
  spdlog
  sqlite3
  wirehair
  wxinclude
  wxtetris
  wxwidgets
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
  openh264
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
    geotiff
    wxx
    )
endif()
set(googletest_deps xpro::gtest_main)
set(activemqcpp_deps xpro::activemqcpp)
set(apr_deps xpro::apr-1)
set(argon2_deps xpro::argon2)
set(azmq_deps xpro::azmq)
set(boost_deps Boost::filesystem Boost::program_options Boost::regex)
set(boost_compile_definitions "BOOST_NO_CXX98_FUNCTION_BASE")
set(bzip2_deps xpro::bz2)
set(cares_deps c-ares::cares)
set(ceres-solver_deps xpro::ceres)
set(cppzmq_deps xpro::cppzmq-static)
set(curl_deps xpro::libcurl)
set(eigen_deps xpro::Eigen)
set(expat_deps xpro::expat)
set(fecpp_deps xpro::fecpp)
set(fmt_deps xpro::fmt)
set(geos_deps xpro::geos_c)
set(geotranz_deps xpro::geotranz)
xpGetPkgVar(geotranz DATA_DIR) # sets GEOTRANZ_DATA_DIR
set(geotranz_compile_definitions "GEOTRANS_DATA=\"${GEOTRANZ_DATA_DIR}\"")
set(glew_deps GLEW::glew_s)
set(hdf5_deps xpro::hdf5_cpp-static)
set(jasper_deps xpro::libjasper)
set(jxrlib_deps xpro::jxrlib)
set(libgit2_deps xpro::git2)
set(libiconv_deps xpro::libiconv)
set(librttopo_deps xpro::librttopo)
set(libsodium_deps xpro::sodium)
set(libspatialite_deps xpro::libspatialite)
set(libssh2_deps xpro::libssh2)
set(libstrophe_deps xpro::libstrophe)
set(libzmq_deps xpro::libzmq-static)
set(libzmq_compile_features cxx_std_17)
set(lua_deps xpro::liblua)
set(nlohmann_json_deps xpro::nlohmann_json)
set(nlohmann_json_compile_features cxx_std_17)
set(openh264_deps xpro::openh264)
set(openssl_deps xpro::crypto xpro::ssl)
set(rapidjson_deps xpro::RapidJSON)
set(rapidxml_deps xpro::rapidxml)
set(shapelib_deps xpro::shape)
set(spdlog_deps xpro::spdlog)
set(spdlog_compile_features cxx_std_17)
set(sqlite3_deps xpro::SQLite3)
set(wirehair_deps xpro::wirehair)
set(wxwidgets_deps wx::base wx::core)
set(zlib_deps xpro::zlibstatic)
set(zmqpp_deps xpro::zmqpp-static)
foreach(pro ${pros})
  find_package(${pro})
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${pro}.cpp)
    add_executable(${pro}_test ${pro}.cpp)
    list(APPEND all_srcs ${pro}.cpp)
    if(DEFINED ${pro}_deps)
      target_link_libraries(${pro}_test PRIVATE ${${pro}_deps} xpro::gtest_main)
      list(APPEND all_deps ${${pro}_deps})
    endif()
    if(DEFINED ${pro}_compile_features)
      target_compile_features(${pro}_test PRIVATE ${${pro}_compile_features})
      list(APPEND all_compile_features ${${pro}_compile_features})
    endif()
    if(DEFINED ${pro}_compile_definitions)
      target_compile_definitions(${pro}_test PRIVATE ${${pro}_compile_definitions})
      list(APPEND all_compile_definitions ${${pro}_compile_definitions})
    endif()
    add_test(NAME ${pro}_test COMMAND ${pro}_test)
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
list(REMOVE_DUPLICATES all_deps)
list(REMOVE_DUPLICATES all_compile_features)
list(REMOVE_DUPLICATES all_compile_definitions)
# TRICKY: libsodium includes argon2, so we need to remove it
# from the lists to avoid duplicate symbols at link-time
list(REMOVE_ITEM all_srcs argon2.cpp)
list(REMOVE_ITEM all_deps ${argon2_deps})
if(TRUE)
  add_executable(toda ${all_srcs})
  target_link_libraries(toda PRIVATE ${all_deps})
  target_compile_features(toda PRIVATE ${all_compile_features})
  target_compile_definitions(toda PRIVATE ${all_compile_definitions})
endif()
xpSourceListAppend()

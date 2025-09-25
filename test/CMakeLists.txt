list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
set(pros
  googletest
  apr
  argon2
  bzip2
  ceres-solver
  eigen
  flatbuffers
  fmt
  geos
  hdf5
  libiconv
  librttopo
  libsodium
  libspatialite
  libzmq
  nasm
  nlohmann_json
  nodeng
  nodexp
  node-addon-api
  nvjpeg2000
  patch
  spatialite-tools
  spdlog
  sqlite3
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
    azmq
    boost
    clangformat
    fecpp
    ffmpeg
    geotiff
    geotrans
    jasper
    jxrlib
    libstrophe
    rapidjson
    wirehair
    wxinclude
    wxwidgets
    wxx
    )
endif()
set(googletest_deps xpro::gtest_main)
set(activemqcpp_deps xpro::activemqcpp xpro::gtest_main)
set(apr_deps xpro::apr-1 xpro::gtest_main)
set(argon2_deps xpro::argon2 xpro::gtest_main)
set(bzip2_deps xpro::bz2 xpro::gtest_main)
set(ceres-solver_deps xpro::ceres xpro::gtest_main)
set(eigen_deps xpro::Eigen xpro::gtest_main)
set(flatbuffers_deps xpro::flatbuffers xpro::gtest_main)
set(flatbuffers_compile_features cxx_std_17)
set(fmt_deps xpro::fmt xpro::gtest_main)
set(geos_deps xpro::geos_c xpro::gtest_main)
set(hdf5_deps xpro::hdf5_cpp-static xpro::gtest_main)
set(libiconv_deps xpro::libiconv xpro::gtest_main)
set(librttopo_deps xpro::librttopo xpro::gtest_main)
set(libsodium_deps xpro::sodium xpro::gtest_main)
set(libspatialite_deps xpro::libspatialite xpro::gtest_main)
set(libzmq_deps xpro::libzmq-static xpro::gtest_main)
set(libzmq_compile_features cxx_std_17)
set(nlohmann_json_deps xpro::nlohmann_json xpro::gtest_main)
set(nlohmann_json_compile_features cxx_std_17)
set(spdlog_deps xpro::spdlog xpro::gtest_main)
set(spdlog_compile_features cxx_std_17)
set(sqlite3_deps xpro::SQLite3 xpro::gtest_main)
set(zlib_deps xpro::zlibstatic xpro::gtest_main)
foreach(pro ${pros})
  find_package(${pro})
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${pro}.cpp)
    add_executable(${pro}_test ${pro}.cpp)
    if(DEFINED ${pro}_deps)
      target_link_libraries(${pro}_test PRIVATE ${${pro}_deps})
    endif()
    if(DEFINED ${pro}_compile_features)
      target_compile_features(${pro}_test PRIVATE ${${pro}_compile_features})
    endif()
    if(DEFINED ${pro}_compile_definitions)
      target_compile_definitions(${pro}_test PRIVATE ${${pro}_compile_definitions})
    endif()
    add_test(NAME ${pro}_test COMMAND ${pro}_test)
  endif()
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${pro}.cmake)
    xpGetPkgVar(${pro} VER)
    xpGetPkgVar(${pro} EXE)
    if(TARGET ${${pro}_EXE})
      set(${pro}_EXE_PATH $<TARGET_FILE:${${pro}_EXE}>)
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
xpSourceListAppend()

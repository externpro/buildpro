list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
set(pros
  googletest
  argon2
  bzip2
  flatbuffers
  fmt
  nlohmann_json
  patch
  spdlog
  zlib
  )
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  list(APPEND pros
    # exdlpro projects
    fecpp
    wxinclude
    )
  list(APPEND pros
    # standalone externpro devel packages
    hdf5
    librttopo
    libspatialite
    nasm
    #node-addon-api
    nodeng
    nvjpeg2000
    spatialite-tools
    )
endif()
set(googletest_deps xpro::gtest_main)
set(argon2_deps xpro::argon2 xpro::gtest_main)
set(bzip2_deps xpro::bz2 xpro::gtest_main)
set(flatbuffers_deps xpro::flatbuffers xpro::gtest_main)
set(flatbuffers_compile_features cxx_std_17)
set(fmt_deps xpro::fmt xpro::gtest_main)
set(nlohmann_json_deps xpro::nlohmann_json xpro::gtest_main)
set(nlohmann_json_compile_features cxx_std_17)
set(spdlog_deps xpro::spdlog xpro::gtest_main)
set(spdlog_compile_features cxx_std_17)
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

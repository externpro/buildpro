list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
set(pros
  googletest
  argon2
  fmt
  spdlog
  )
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  list(APPEND pros
    exdlpro
    # exdlpro projects
    fecpp
    wxinclude
    )
  list(APPEND pros
    hdf5
    librttopo
    libspatialite
    nasm
    #node-addon-api
    nodeng
    nvjpeg2000
    patch
    spatialite-tools
    )
endif()
set(googletest_deps xpro::gtest_main)
set(argon2_deps xpro::argon2 xpro::gtest_main)
set(fmt_deps xpro::fmt xpro::gtest_main)
set(spdlog_deps xpro::spdlog xpro::gtest_main)
set(spdlog_compile_features cxx_std_17)
foreach(pro ${pros})
  xpFindPkg(PKGS ${pro})
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
endforeach()
xpSourceListAppend()

xpGetPkgVar(spatialite-tools VER)
xpFindPkg(PKGS libspatialite sqlite3)
xpGetPkgVar(libspatialite VER)
xpGetPkgVar(sqlite3 VER)
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

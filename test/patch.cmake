set(pro patch)
set(cmd "${${pro}_EXE} --version")
execute_process(COMMAND bash -c "${cmd} 2>&1 | head -n 1"
  ERROR_VARIABLE ${pro}_err ERROR_STRIP_TRAILING_WHITESPACE
  OUTPUT_VARIABLE ${pro}_out OUTPUT_STRIP_TRAILING_WHITESPACE
  RESULT_VARIABLE result
  )
if(NOT result EQUAL 0)
  message(FATAL_ERROR "${cmd} failed with exit code ${result}")
endif()
if(${pro}_out)
  set(${pro}_ver ${${pro}_out})
elseif(${pro}_err)
  set(${pro}_ver ${${pro}_err})
endif()
string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" ${pro}_ver3 ${${pro}_ver})
string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" ${pro}_VER3 ${${pro}_VER})
message(STATUS "cmake ${pro} version: [${${pro}_VER3}] ${${pro}_VER}")
message(STATUS "executable ${pro} version: [${${pro}_ver3}] ${${pro}_ver}")
if(${pro}_VER3 VERSION_EQUAL ${pro}_ver3)
  message(STATUS "${pro} version match: ${${pro}_VER3} == ${${pro}_ver3}")
else()
  message(FATAL_ERROR "${pro} version mismatch: ${${pro}_VER3} != ${${pro}_ver3}")
endif()

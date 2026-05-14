# Function to extract just the numeric version part
function(extract_numeric_version output_var version_string)
  # Extract just the X.Y.Z part, ignoring any suffix
  # Note: Double backslashes are needed to escape the dots in the regex pattern
  string(REGEX MATCH "([0-9]+\\.[0-9]+\\.[0-9]+)" numeric_version "${version_string}")
  if(numeric_version)
    set(${output_var} ${CMAKE_MATCH_1} PARENT_SCOPE)
  else()
    set(${output_var} "${version_string}" PARENT_SCOPE)  # Return original if no match
  endif()
endfunction()
# Get the expected versions
extract_numeric_version(spatialite-tools_VER3 "@spatialite-tools_VER@")
extract_numeric_version(libspatialite_VER3 "@libspatialite_VER@")
extract_numeric_version(SQLite3_VER3 "@SQLite3_VER@")
# Get the version from the tool
set(cmd "@tool_EXE@ --version")
execute_process(COMMAND bash -c "${cmd} 2>&1 | head -n 8"
  ERROR_VARIABLE err ERROR_STRIP_TRAILING_WHITESPACE
  OUTPUT_VARIABLE out OUTPUT_STRIP_TRAILING_WHITESPACE
  RESULT_VARIABLE result
  )
if(NOT result EQUAL 0)
  message(FATAL_ERROR "${cmd} failed with exit code ${result}")
endif()
if(out)
  set(ver ${out})
elseif(err)
  set(ver ${err})
endif()
# Get the tool name from the executable path
get_filename_component(tool_name "@tool_EXE@" NAME_WE)
# Special case for spatialite, who's version output is different
if(tool_name STREQUAL "spatialite")
  # Extract the version number from the beginning of the output
  # Example: "3.38.2 2022-03-26 13:51:10 d33c709cc0..." -> "3.38.2"
  string(REGEX MATCH "^([0-9]+\\.[0-9]+\\.[0-9]+)" matched "${ver}")
  if(NOT matched)
    message(FATAL_ERROR "Could not extract version from spatialite output: ${ver}")
  endif()
  set(tool_version_num ${CMAKE_MATCH_1})
  message(STATUS "Tool (${tool_name}) version: ${tool_version_num}, expected to match SQLite3 @SQLite3_VER@, will compare 3 digit versions")
  # Compare with the SQLite3 version
  if(NOT "${tool_version_num}" VERSION_EQUAL "${SQLite3_VER3}")
    message(FATAL_ERROR "Tool (${tool_name}) version (${tool_version_num}) does not match SQLite3 version (${SQLite3_VER3})")
  else()
    message(STATUS "Tool (${tool_name}) version match: ${tool_version_num} == ${SQLite3_VER3}")
  endif()
  return()
# Special case for spatialite_tool which shows up as exif_loader in version output
elseif(tool_name STREQUAL "spatialite_tool")
  set(tool_name "exif_loader")
endif()
# Function to find and extract version from a pattern
function(extract_version output_var pattern)
  # Try to match the pattern at the start of a line, followed by optional spaces and dots, then a colon and version
  string(REGEX MATCH "${pattern}[ .:]+([0-9]+\\.[0-9]+\\.[0-9]+[a-z]?)" matched "${ver}")
  if(matched)
    set(${output_var} ${CMAKE_MATCH_1} PARENT_SCOPE)
  else()
    # Try alternative pattern with different spacing
    string(REGEX MATCH "${pattern}[ ]*:[ ]*([0-9]+\\.[0-9]+\\.[0-9]+[a-z]?)" matched "${ver}")
    if(matched)
      set(${output_var} ${CMAKE_MATCH_1} PARENT_SCOPE)
    else()
      message(STATUS "WARNING: Could not find version for pattern '${pattern}' in:\n${ver}")
      set(${output_var} "NOT_FOUND" PARENT_SCOPE)
    endif()
  endif()
endfunction()
# Extract versions using patterns
extract_version(tool_version "${tool_name}")
extract_version(libspatialite_version "libspatialite")
extract_version(SQLite3_version "libsqlite3")
# Output the extracted versions and expected versions
message(STATUS "Tool (${tool_name}) version: ${tool_version}, expected @spatialite-tools_VER@, will compare 3 digit versions")
message(STATUS "libspatialite version: ${libspatialite_version}, expected @libspatialite_VER@, will compare 3 digit versions")
message(STATUS "SQLite3 version: ${SQLite3_version}, expected @SQLite3_VER@, will compare 3 digit versions")
# Also extract numeric parts from the actual versions for comparison
extract_numeric_version(tool_version_num "${tool_version}")
extract_numeric_version(libspatialite_version_num "${libspatialite_version}")
extract_numeric_version(SQLite3_version_num "${SQLite3_version}")
# Compare with expected versions using numeric versions and 3 digit versions only
if(NOT "${tool_version_num}" VERSION_EQUAL "${spatialite-tools_VER3}")
  message(FATAL_ERROR "Tool (${tool_name}) version mismatch: expected ${spatialite-tools_VER3} (got ${tool_version_num})")
else()
  message(STATUS "Tool (${tool_name}) version match: ${tool_version_num} == ${spatialite-tools_VER3}")
endif()
if(NOT "${libspatialite_version_num}" VERSION_EQUAL "${libspatialite_VER3}")
  message(FATAL_ERROR "libspatialite version mismatch: expected ${libspatialite_VER3} (got ${libspatialite_version_num})")
else()
  message(STATUS "libspatialite version match: ${libspatialite_version_num} == ${libspatialite_VER3}")
endif()
if(NOT "${SQLite3_version_num}" VERSION_EQUAL "${SQLite3_VER3}")
  message(FATAL_ERROR "SQLite3 version mismatch: expected ${SQLite3_VER3} (got ${SQLite3_version_num})")
else()
  message(STATUS "SQLite3 version match: ${SQLite3_version_num} == ${SQLite3_VER3}")
endif()

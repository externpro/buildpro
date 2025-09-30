cmake_minimum_required(VERSION 3.10)
project(MSVCInfo LANGUAGES CXX)
# Get CMake version information
message(STATUS "CMAKE_VERSION: ${CMAKE_VERSION}")
# Output MSVC information if available
if(MSVC)
  message(STATUS "MSVC_VERSION: ${MSVC_VERSION}")
  message(STATUS "MSVC_TOOLSET_VERSION: [vc]${MSVC_TOOLSET_VERSION}")
  # Additional MSVC information
  message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
  message(STATUS "CMAKE_CXX_COMPILER_VERSION: ${CMAKE_CXX_COMPILER_VERSION}")
  message(STATUS "MSVC_IDE: ${MSVC_IDE}")
else()
  message(STATUS "Not using MSVC compiler")
  message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
  message(STATUS "CMAKE_CXX_COMPILER_VERSION: ${CMAKE_CXX_COMPILER_VERSION}")
endif()

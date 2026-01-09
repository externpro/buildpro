set(tgt wxinc_hrc_test)
set(depsDot ${CMAKE_SOURCE_DIR}/.devcontainer/cmake/deps.dot)
set(graphDot ${CMAKE_SOURCE_DIR}/.devcontainer/graph/graph.dot)
set(depsSvg ${CMAKE_CURRENT_BINARY_DIR}/deps.svg)
set(graphPng ${CMAKE_CURRENT_BINARY_DIR}/graph.png)
find_program(DOT_EXE dot)
if(NOT DOT_EXE)
  message(FATAL_ERROR "dot executable not found")
endif()
add_custom_command(OUTPUT ${depsSvg}
  COMMAND ${DOT_EXE} -Tsvg -o${depsSvg} ${depsDot}
  DEPENDS ${depsDot}
  VERBATIM
  )
add_custom_command(OUTPUT ${graphPng}
  COMMAND ${DOT_EXE} -Tpng -o${graphPng} ${graphDot}
  DEPENDS ${graphDot}
  VERBATIM
  )
add_custom_target(wxinc_hrc_graphs DEPENDS ${depsSvg} ${graphPng})
xpCreateHeaderResource(hrcList
  ${depsSvg}
  ${graphPng}
  )
add_executable(${tgt} hrc.cpp ${hrcList})
add_dependencies(${tgt} wxinc_hrc_graphs)
target_include_directories(${tgt} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_compile_definitions(${tgt} PRIVATE DEPS_SVG_PATH="${depsSvg}" GRAPH_PNG_PATH="${graphPng}")
target_link_libraries(${tgt} PRIVATE xpro::gtest_main)
add_test(NAME ${tgt} COMMAND ${tgt})

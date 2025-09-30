set(tgt wxinc_hrc_test)
xpCreateHeaderResource(hrcList
  ${CMAKE_SOURCE_DIR}/.devcontainer/cmake/deps.png
  ${CMAKE_SOURCE_DIR}/.devcontainer/graph/graph.png
  )
add_executable(${tgt} hrc.cpp ${hrcList})
target_include_directories(${tgt} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_compile_definitions(${tgt} PRIVATE CMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
target_link_libraries(${tgt} PRIVATE xpro::gtest_main)
add_test(NAME ${tgt} COMMAND ${tgt})

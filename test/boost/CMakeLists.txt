add_executable(boost_iostreams_test iostreams.cpp)
target_link_libraries(boost_iostreams_test PRIVATE
  Boost::filesystem Boost::iostreams
  xpro::bz2 xpro::zlibstatic xpro::gtest_main
  )
add_test(NAME boost_iostreams_test COMMAND boost_iostreams_test)

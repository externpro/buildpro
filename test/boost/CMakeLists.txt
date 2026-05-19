add_executable(boost_iostreams_test iostreams.cpp)
target_link_libraries(boost_iostreams_test PRIVATE Boost::filesystem Boost::iostreams googletest)
add_test(NAME boost_iostreams_test COMMAND boost_iostreams_test)

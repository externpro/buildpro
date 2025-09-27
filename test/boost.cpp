#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <gtest/gtest.h>

namespace po = boost::program_options;

// Test for Boost::program_options
TEST(BoostProgramOptionsTest, BasicFunctionality)
{
  // Create options description
  po::options_description desc("Test options");

  // Add some options
  desc.add_options()("help,h", "produce help message")(
    "compression,c",
    po::value<int>()->default_value(6),
    "compression level (0-9)")(
    "input-file,i", po::value<std::string>(), "input file")(
    "verbose,v", po::value<bool>()->default_value(false), "verbose output");

  // Parse command line arguments
  const char* args[] = {
    "program", "--compression=9", "--input-file=test.txt", "--verbose=true"};
  int argc = sizeof(args) / sizeof(args[0]);

  po::variables_map vm;
  po::store(po::parse_command_line(argc, const_cast<char**>(args), desc), vm);
  po::notify(vm);

  // Test that options were correctly parsed
  EXPECT_TRUE(vm.count("compression"));
  EXPECT_EQ(vm["compression"].as<int>(), 9);

  EXPECT_TRUE(vm.count("input-file"));
  EXPECT_EQ(vm["input-file"].as<std::string>(), "test.txt");

  EXPECT_TRUE(vm.count("verbose"));
  EXPECT_TRUE(vm["verbose"].as<bool>());
}

// Test for Boost::filesystem
TEST(BoostFilesystemTest, PathOperations)
{
  // Create a platform-independent path
  boost::filesystem::path p;

#ifdef _WIN32
  // Windows-style path
  p = "C:\\Program Files\\include";

  EXPECT_EQ(p.filename().string(), "include");
  EXPECT_EQ(p.parent_path().string(), "C:\\Program Files");

  boost::filesystem::path p2 = p / "boost";
  // Use generic_string() for platform-independent path representation
  EXPECT_EQ(p2.generic_string(), "C:/Program Files/include/boost");
#else
  // Unix-style path
  p = "/usr/local/include";

  EXPECT_EQ(p.string(), "/usr/local/include");
  EXPECT_EQ(p.filename().string(), "include");
  EXPECT_EQ(p.parent_path().string(), "/usr/local");

  boost::filesystem::path p2 = p / "boost";
  EXPECT_EQ(p2.string(), "/usr/local/include/boost");
#endif

  // Test platform-independent operations
  boost::filesystem::path generic_path("dir/subdir/file.txt");
  EXPECT_EQ(generic_path.filename().string(), "file.txt");
  EXPECT_EQ(generic_path.stem().string(), "file");
  EXPECT_EQ(generic_path.extension().string(), ".txt");
}

// Test for Boost::algorithm::string
TEST(BoostAlgorithmTest, StringOperations)
{
  std::string str = "  Hello, Boost World!  ";

  // Test trim
  std::string trimmed = str;
  boost::algorithm::trim(trimmed);
  EXPECT_EQ(trimmed, "Hello, Boost World!");

  // Test to_upper
  std::string upper = str;
  boost::algorithm::to_upper(upper);
  EXPECT_EQ(upper, "  HELLO, BOOST WORLD!  ");

  // Test split
  std::vector<std::string> tokens;
  boost::algorithm::split(
    tokens, trimmed, boost::is_any_of(", "), boost::token_compress_on);
  ASSERT_EQ(tokens.size(), 3);
  EXPECT_EQ(tokens[0], "Hello");
  EXPECT_EQ(tokens[1], "Boost");
  EXPECT_EQ(tokens[2], "World!");
}

// Test for Boost::regex
TEST(BoostRegexTest, RegexMatching)
{
  boost::regex expr("\\w+\\s\\w+");

  EXPECT_TRUE(boost::regex_match("Hello World", expr));
  EXPECT_FALSE(boost::regex_match("HelloWorld", expr));

  std::string s = "Boost is a collection of libraries";
  boost::smatch what;
  boost::regex_search(s, what, boost::regex("\\w+ is a"));

  ASSERT_EQ(what.size(), 1);
  EXPECT_EQ(what[0], "Boost is a");
}

// Test for Boost::format
TEST(BoostFormatTest, Formatting)
{
  boost::format fmt("Hello, %1%! You are visitor #%2%.");
  fmt % "World" % 42;

  EXPECT_EQ(fmt.str(), "Hello, World! You are visitor #42.");
}

// Test for Boost::lexical_cast
TEST(BoostLexicalCastTest, TypeConversion)
{
  int i = boost::lexical_cast<int>("42");
  EXPECT_EQ(i, 42);

  std::string s = boost::lexical_cast<std::string>(42);
  EXPECT_EQ(s, "42");

  double d = boost::lexical_cast<double>("3.14159");
  EXPECT_DOUBLE_EQ(d, 3.14159);

  EXPECT_THROW(
    boost::lexical_cast<int>("not a number"), boost::bad_lexical_cast);
}

// Test for Boost::uuid
TEST(BoostUuidTest, UuidGeneration)
{
  boost::uuids::random_generator gen;
  boost::uuids::uuid id1 = gen();
  boost::uuids::uuid id2 = gen();

  // UUIDs should be different
  EXPECT_NE(id1, id2);

  // UUID should not be nil
  EXPECT_FALSE(id1.is_nil());

  // Test nil UUID
  boost::uuids::uuid nil_id = boost::uuids::nil_uuid();
  EXPECT_TRUE(nil_id.is_nil());

  // Test string conversion
  std::string id_str = boost::uuids::to_string(id1);
  EXPECT_EQ(id_str.length(), 36); // 32 hex digits + 4 hyphens
}

// Complex test combining multiple Boost libraries
TEST(BoostCombinedTest, MultipleLibraries)
{
  // Create a command line parser
  po::options_description desc("Test options");
  desc.add_options()("input-file", po::value<std::string>(), "input file")(
    "output-file", po::value<std::string>(), "output file")(
    "compression",
    po::value<int>()->default_value(6),
    "compression level (0-9)");

  // Simulate command line arguments
  const char* args[] = {
    "program", "--input-file", "data.txt", "--compression", "8"};
  int argc = sizeof(args) / sizeof(args[0]);

  po::variables_map vm;
  po::store(po::parse_command_line(argc, const_cast<char**>(args), desc), vm);
  po::notify(vm);

  // Extract values
  std::string input_file = vm["input-file"].as<std::string>();
  int compression = vm["compression"].as<int>();

  // Use boost::filesystem to work with the path
  boost::filesystem::path input_path(input_file);
  EXPECT_EQ(input_path.filename().string(), "data.txt");

  // Use boost::format to create a message
  boost::format msg("Processing %1% with compression level %2%");
  msg % input_file % compression;

  std::string message = msg.str();
  EXPECT_EQ(message, "Processing data.txt with compression level 8");

  // Use boost::algorithm to modify the message
  boost::algorithm::to_upper(message);
  EXPECT_EQ(message, "PROCESSING DATA.TXT WITH COMPRESSION LEVEL 8");
}

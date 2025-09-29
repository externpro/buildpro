#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#pragma warning(push)
// conversion from std::streamsize to int, possible loss of data
#pragma warning(disable : 4244)
#include <boost/iostreams/copy.hpp>
#pragma warning(pop)
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/stream.hpp>

#include <gtest/gtest.h>

namespace bfs = boost::filesystem;
namespace bio = boost::iostreams;

// Helper function to create a temporary file with given content and extension
std::string createTempFile(const std::string& content,
                           const std::string& extension)
{
  // Create a temporary file path
  bfs::path tempPath = bfs::temp_directory_path() / bfs::unique_path();
  tempPath += extension;

  // Write content to the file
  std::ofstream file(
    tempPath.string(), std::ios_base::out | std::ios_base::binary);
  file.write(content.c_str(), content.size());
  file.close();

  return tempPath.string();
}

// Helper function to compress data using bzip2
std::string compressBzip2(const std::string& data)
{
  std::string compressed;
  bio::filtering_streambuf<bio::output> out;
  out.push(bio::bzip2_compressor());
  out.push(bio::back_inserter(compressed));
  bio::write(out, data.data(), data.size());
  bio::close(out);
  return compressed;
}

// Helper function to compress data using zlib
std::string compressZlib(const std::string& data)
{
  std::string compressed;
  bio::filtering_streambuf<bio::output> out;
  out.push(bio::zlib_compressor());
  out.push(bio::back_inserter(compressed));
  bio::write(out, data.data(), data.size());
  bio::close(out);
  return compressed;
}

// Helper function to decompress data using boost::iostreams
std::string decompressFile(const std::string& filePath)
{
  bfs::path filepath(filePath);
  std::string ext = filepath.extension().string();

  bio::filtering_streambuf<bio::input> in;
  if (ext.compare(".bz2") == 0)
  {
    in.push(bio::bzip2_decompressor());
  }
  else if (ext.compare(".Z") == 0)
  {
    in.push(bio::zlib_decompressor());
  }
  else
  {
    throw std::runtime_error("Unsupported extension (must be .bz2 or .Z)");
  }

  std::ifstream file(filePath, std::ios_base::in | std::ios_base::binary);
  in.push(file);

  std::stringstream decompressed;
  bio::copy(in, decompressed);

  return decompressed.str();
}

// Test fixture for decompression tests
class IostreamsTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Test messages
    testMessage =
      "Hello, this is a test message for compression and decompression!";
    longTestMessage = std::string(10000, 'A') + std::string(10000, 'B') +
                      std::string(10000, 'C');
  }

  void TearDown() override
  {
    // Clean up any temporary files
    for (const auto& path : tempFiles)
    {
      if (bfs::exists(path))
      {
        bfs::remove(path);
      }
    }
  }

  // Helper to add a temp file to the cleanup list
  void addTempFile(const std::string& path) { tempFiles.push_back(path); }

  std::string testMessage;
  std::string longTestMessage;
  std::vector<std::string> tempFiles;
};

// Test bzip2 compression and decompression
TEST_F(IostreamsTest, Bzip2CompressionDecompression)
{
  // Compress the test message
  std::string compressed = compressBzip2(testMessage);

  // Write compressed data to a temporary file
  std::string tempFilePath = createTempFile(compressed, ".bz2");
  addTempFile(tempFilePath);

  // Decompress the file
  std::string decompressed = decompressFile(tempFilePath);

  // Verify the decompressed content matches the original
  EXPECT_EQ(testMessage, decompressed);
}

// Test zlib compression and decompression
TEST_F(IostreamsTest, ZlibCompressionDecompression)
{
  // Compress the test message
  std::string compressed = compressZlib(testMessage);

  // Write compressed data to a temporary file
  std::string tempFilePath = createTempFile(compressed, ".Z");
  addTempFile(tempFilePath);

  // Decompress the file
  std::string decompressed = decompressFile(tempFilePath);

  // Verify the decompressed content matches the original
  EXPECT_EQ(testMessage, decompressed);
}

// Test bzip2 compression and decompression with a large message
TEST_F(IostreamsTest, Bzip2LargeCompressionDecompression)
{
  // Compress the long test message
  std::string compressed = compressBzip2(longTestMessage);

  // Write compressed data to a temporary file
  std::string tempFilePath = createTempFile(compressed, ".bz2");
  addTempFile(tempFilePath);

  // Decompress the file
  std::string decompressed = decompressFile(tempFilePath);

  // Verify the decompressed content matches the original
  EXPECT_EQ(longTestMessage, decompressed);
}

// Test zlib compression and decompression with a large message
TEST_F(IostreamsTest, ZlibLargeCompressionDecompression)
{
  // Compress the long test message
  std::string compressed = compressZlib(longTestMessage);

  // Write compressed data to a temporary file
  std::string tempFilePath = createTempFile(compressed, ".Z");
  addTempFile(tempFilePath);

  // Decompress the file
  std::string decompressed = decompressFile(tempFilePath);

  // Verify the decompressed content matches the original
  EXPECT_EQ(longTestMessage, decompressed);
}

// Test error handling for unsupported file extensions
TEST_F(IostreamsTest, UnsupportedExtension)
{
  // Create a file with an unsupported extension
  std::string tempFilePath = createTempFile("Some content", ".txt");
  addTempFile(tempFilePath);

  // Attempt to decompress should throw an exception
  EXPECT_THROW(decompressFile(tempFilePath), std::runtime_error);
}

// Test error handling for corrupted bzip2 compressed files
TEST_F(IostreamsTest, CorruptedBzip2CompressedFile)
{
  // Create a corrupted bzip2 file (just random data with .bz2 extension)
  std::string tempFilePath =
    createTempFile("This is not valid bzip2 data", ".bz2");
  addTempFile(tempFilePath);

  // Attempt to decompress should throw an exception
  EXPECT_THROW(decompressFile(tempFilePath), bio::bzip2_error);
}

// Test error handling for corrupted zlib compressed files
TEST_F(IostreamsTest, CorruptedZlibCompressedFile)
{
  // Create a corrupted zlib file (just random data with .Z extension)
  std::string tempFilePath =
    createTempFile("This is not valid zlib data", ".Z");
  addTempFile(tempFilePath);

  // Attempt to decompress should throw an exception
  EXPECT_THROW(decompressFile(tempFilePath), bio::zlib_error);
}

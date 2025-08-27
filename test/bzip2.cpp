#include <algorithm>
#include <random>
#include <string>
#include <vector>

#include <bzip2/bzlib.h>
#include <gtest/gtest.h>

class BZip2Test : public ::testing::Test
{
protected:
  std::vector<char> test_data;
  const size_t TEST_DATA_SIZE = 1024 * 1024; // 1MB test data

  void SetUp() override
  {
    // Generate compressible test data (repeating pattern)
    test_data.resize(TEST_DATA_SIZE);
    const std::string pattern = "This is a test pattern that will be repeated "
                                "to create compressible data. ";

    for (size_t i = 0; i < test_data.size(); ++i)
    {
      test_data[i] = pattern[i % pattern.size()];
    }
  }

  std::vector<char> compress(const std::vector<char>& input,
                             int blockSize100k = 9)
  {
    // Handle empty input
    if (input.empty())
    {
      return {};
    }

    unsigned int in_size = static_cast<unsigned int>(input.size());
    unsigned int out_size =
      in_size + (in_size / 100) + 600; // Standard bzip2 overhead
    std::vector<char> output(out_size, 0);

    int result = BZ2_bzBuffToBuffCompress(output.data(),
                                          &out_size,
                                          const_cast<char*>(input.data()),
                                          in_size,
                                          blockSize100k,
                                          0, // verbosity (0 = silent)
                                          30 // work factor (default is 30)
    );

    if (result != BZ_OK && !(result == BZ_OUTBUFF_FULL && in_size == 0))
    {
      throw std::runtime_error("Compression failed with error: " +
                               std::to_string(result));
    }

    output.resize(out_size);
    return output;
  }

  std::vector<char> decompress(const std::vector<char>& input,
                               size_t original_size)
  {
    // Handle empty input
    if (input.empty() && original_size == 0)
    {
      return {};
    }

    if (input.empty())
    {
      throw std::runtime_error(
        "Cannot decompress empty input to non-zero size");
    }

    std::vector<char> output(original_size, 0);
    unsigned int out_size = static_cast<unsigned int>(original_size);

    int result =
      BZ2_bzBuffToBuffDecompress(output.data(),
                                 &out_size,
                                 const_cast<char*>(input.data()),
                                 static_cast<unsigned int>(input.size()),
                                 0, // small
                                 0  // verbosity
      );

    if (result != BZ_OK)
    {
      throw std::runtime_error("Decompression failed with error: " +
                               std::to_string(result));
    }

    output.resize(out_size);
    return output;
  }
};

TEST_F(BZip2Test, BasicCompressionDecompression)
{
  // Compress the test data
  auto compressed = compress(test_data);

  // Check if compression was effective
  // We expect at least 50% compression for our test pattern
  EXPECT_LT(compressed.size(), test_data.size() * 0.5);

  // Decompress back
  auto decompressed = decompress(compressed, test_data.size());

  // Verify decompressed data matches original
  EXPECT_EQ(test_data, decompressed);
}

TEST_F(BZip2Test, EmptyInput)
{
  std::vector<char> empty;
  auto compressed = compress(empty);
  EXPECT_TRUE(compressed.empty()); // Should return empty vector for empty input

  // Test decompressing empty input
  auto decompressed = decompress(compressed, 0);
  EXPECT_TRUE(decompressed.empty());
}

TEST_F(BZip2Test, SmallInput)
{
  std::string small_data = "The quick brown fox jumps over the lazy dog";
  std::vector<char> input(small_data.begin(), small_data.end());

  auto compressed = compress(input);
  auto decompressed = decompress(compressed, input.size());

  std::string result(decompressed.begin(), decompressed.end());
  EXPECT_EQ(small_data, result);
}

TEST_F(BZip2Test, CompressionLevels)
{
  // Test different compression levels (1-9)
  for (int level = 1; level <= 9; ++level)
  {
    auto compressed = compress(test_data, level);
    auto decompressed = decompress(compressed, test_data.size());
    EXPECT_EQ(test_data, decompressed);
  }
}

TEST_F(BZip2Test, ErrorHandling)
{
  // Test invalid compression level
  std::vector<char> output(100);
  unsigned int out_len = 100;
  const char* input = "test";
  unsigned int in_len = 4;

  // Invalid compression level
  int result = BZ2_bzBuffToBuffCompress(
    output.data(), &out_len, const_cast<char*>(input), in_len, 10, 0, 0);
  EXPECT_NE(result, BZ_OK);

  // Invalid buffer for decompression
  char bad_compressed[] = "not a valid bzip2 stream";
  char decompressed[100];
  unsigned int decompressed_size = 100;
  result = BZ2_bzBuffToBuffDecompress(decompressed,
                                      &decompressed_size,
                                      bad_compressed,
                                      sizeof(bad_compressed),
                                      0,
                                      0);
  EXPECT_NE(result, BZ_OK);
}

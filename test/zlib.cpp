#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <zlib.h>

TEST(ZlibTest, CompressAndDecompress)
{
  // Original data
  const std::string original =
    "This is a test string to compress and decompress using zlib";

  // Compression
  uLongf compressedSize = compressBound(original.size());
  std::vector<Bytef> compressed(compressedSize);

  int result = compress2(compressed.data(),
                         &compressedSize,
                         reinterpret_cast<const Bytef*>(original.data()),
                         original.size(),
                         Z_BEST_COMPRESSION);

  ASSERT_EQ(Z_OK, result) << "Compression failed";

  // Decompression
  std::string decompressed(original.size(), '\0');
  uLongf decompressedSize = decompressed.size();
  std::vector<Bytef> decompressBuffer(decompressedSize);

  result = uncompress(decompressBuffer.data(),
                      &decompressedSize,
                      compressed.data(),
                      compressedSize);

  ASSERT_EQ(Z_OK, result) << "Decompression failed";
  decompressed.assign(
    reinterpret_cast<const char*>(decompressBuffer.data()), decompressedSize);

  // Verify the decompressed data matches the original
  EXPECT_EQ(original, decompressed);
}

TEST(ZlibTest, Version)
{
  EXPECT_NE(nullptr, zlibVersion());
  std::cout << "Using zlib version: " << zlibVersion() << std::endl;
}

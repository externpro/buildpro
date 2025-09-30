#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

// Include the generated header resource files
#include "Resources/deps.hrc"
#include "Resources/graph.hrc"

/**
 * Helper function to read binary file into vector
 *
 * @param filename Path to the file to read
 * @return Vector containing file contents or empty vector if file can't be read
 */
std::vector<unsigned char> readBinaryFile(const std::string& filename)
{
  std::ifstream file(filename, std::ios::binary);
  if (!file)
  {
    std::cerr << "Error: Could not open file " << filename << std::endl;
    return {}; // Return empty vector if file can't be opened
  }

  // Get file size
  file.seekg(0, std::ios::end);
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  // Read file content into vector
  std::vector<unsigned char> buffer(size);
  if (file.read(reinterpret_cast<char*>(buffer.data()), size))
  {
    return buffer;
  }

  std::cerr << "Error: Could not read file " << filename << std::endl;
  return {};
}

/**
 * Helper function to print a hex dump of data for debugging
 *
 * @param data Pointer to the data
 * @param size Size of the data in bytes
 * @param bytesPerLine Number of bytes to display per line
 */
void printHexDump(const unsigned char* data,
                  size_t size,
                  size_t bytesPerLine = 16)
{
  for (size_t i = 0; i < size; i += bytesPerLine)
  {
    std::cout << std::setw(4) << std::setfill('0') << std::hex << i << ": ";

    // Print hex values
    for (size_t j = 0; j < bytesPerLine; ++j)
    {
      if (i + j < size)
      {
        std::cout << std::setw(2) << std::setfill('0') << std::hex
                  << static_cast<int>(data[i + j]) << " ";
      }
      else
      {
        std::cout << "   ";
      }
    }

    std::cout << " ";

    // Print ASCII representation
    for (size_t j = 0; j < bytesPerLine; ++j)
    {
      if (i + j < size)
      {
        unsigned char c = data[i + j];
        if (c >= 32 && c <= 126)
        { // Printable ASCII
          std::cout << c;
        }
        else
        {
          std::cout << ".";
        }
      }
    }

    std::cout << std::endl;
  }
  std::cout << std::dec; // Reset to decimal output
}

/**
 * Test fixture for header resource tests
 */
class HeaderResourceTest : public ::testing::Test
{
protected:
  /**
   * Compare a binary file with an embedded resource
   *
   * @param filePath Path to the original file
   * @param embeddedData Pointer to the embedded data
   * @param embeddedSize Size of the embedded data
   * @param resourceName Name of the resource for error messages
   * @return true if files match, false otherwise
   */
  bool compareFileWithResource(const std::string& filePath,
                               const unsigned char* embeddedData,
                               size_t embeddedSize,
                               const std::string& resourceName)
  {
    // Read the original file
    std::vector<unsigned char> originalContent = readBinaryFile(filePath);
    if (originalContent.empty())
    {
      ADD_FAILURE() << "Failed to read original file: " << filePath;
      return false;
    }

    // Compare sizes
    if (originalContent.size() != embeddedSize)
    {
      ADD_FAILURE() << "Size mismatch for " << resourceName << ": "
                    << "Original: " << originalContent.size() << " bytes, "
                    << "Embedded: " << embeddedSize << " bytes";
      return false;
    }

    // Compare content
    bool match = true;
    size_t mismatchCount = 0;
    size_t firstMismatch = 0;

    for (size_t i = 0; i < embeddedSize; ++i)
    {
      if (originalContent[i] != embeddedData[i])
      {
        if (mismatchCount == 0)
        {
          firstMismatch = i;
        }
        mismatchCount++;
        match = false;
      }
    }

    // Report mismatches if any
    if (!match)
    {
      ADD_FAILURE() << "Content mismatch for " << resourceName << ": "
                    << mismatchCount << " bytes differ. "
                    << "First mismatch at byte " << firstMismatch;

      // Print hex dump of the area around the first mismatch
      const size_t contextSize = 16;
      size_t start =
        (firstMismatch > contextSize) ? firstMismatch - contextSize : 0;
      size_t end = std::min(firstMismatch + contextSize, embeddedSize);
      size_t length = end - start;

      std::cout << "\nOriginal file bytes around mismatch:" << std::endl;
      printHexDump(&originalContent[start], length);

      std::cout << "\nEmbedded resource bytes around mismatch:" << std::endl;
      printHexDump(&embeddedData[start], length);
    }

    return match;
  }
};

// NOTE: CMAKE_SOURCE_DIR is defined via CMake

TEST_F(HeaderResourceTest, DepsPngMatch)
{
  // Path to the original PNG file
  std::string pngPath =
    std::string(CMAKE_SOURCE_DIR) + "/.devcontainer/cmake/deps.png";

  // Compare the original file with the embedded resource
  EXPECT_TRUE(
    compareFileWithResource(pngPath, deps_png, sizeof(deps_png), "deps.png"));
}

TEST_F(HeaderResourceTest, GraphPngMatch)
{
  // Path to the original PNG file
  std::string pngPath =
    std::string(CMAKE_SOURCE_DIR) + "/.devcontainer/graph/graph.png";

  // Compare the original file with the embedded resource
  EXPECT_TRUE(compareFileWithResource(
    pngPath, graph_png, sizeof(graph_png), "graph.png"));
}

/**
 * Test to verify the structure of the generated header files
 */
TEST_F(HeaderResourceTest, VerifyHeaderStructure)
{
  // Check that the arrays have valid addresses (will always be true, but kept
  // for documentation)
  EXPECT_NE(reinterpret_cast<const void*>(deps_png), nullptr)
    << "deps_png array is not defined";
  EXPECT_NE(reinterpret_cast<const void*>(graph_png), nullptr)
    << "graph_png array is not defined";

  // Check that the arrays have reasonable sizes (PNG files should be at least
  // 100 bytes)
  EXPECT_GT(sizeof(deps_png), 100u) << "deps_png array is too small";
  EXPECT_GT(sizeof(graph_png), 100u) << "graph_png array is too small";

  // Check PNG file signature (first 8 bytes of a PNG file)
  const unsigned char pngSignature[] = {
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

  // Check deps.png signature
  for (size_t i = 0; i < 8; ++i)
  {
    EXPECT_EQ(deps_png[i], pngSignature[i])
      << "deps_png does not have a valid PNG signature at byte " << i;
  }

  // Check graph.png signature
  for (size_t i = 0; i < 8; ++i)
  {
    EXPECT_EQ(graph_png[i], pngSignature[i])
      << "graph_png does not have a valid PNG signature at byte " << i;
  }
}

/**
 * Test to verify that the resource files are properly structured
 * and contain the expected IHDR chunk (second chunk in a PNG file)
 */
TEST_F(HeaderResourceTest, VerifyPngStructure)
{
  // PNG files have chunks that follow the signature
  // Each chunk has: length (4 bytes) + type (4 bytes) + data (length bytes) +
  // CRC (4 bytes) The first chunk should be IHDR (Image Header)

  // First, check deps.png
  // Skip the 8-byte signature
  const unsigned char* chunk = deps_png + 8;

  // Get the chunk length (big-endian)
  uint32_t length =
    (chunk[0] << 24) | (chunk[1] << 16) | (chunk[2] << 8) | chunk[3];

  // Check that the length is reasonable (IHDR is 13 bytes)
  EXPECT_EQ(length, 13u) << "deps.png IHDR chunk has unexpected length";

  // Check the chunk type (should be "IHDR")
  EXPECT_EQ(chunk[4], 'I') << "deps.png first chunk is not IHDR (byte 0)";
  EXPECT_EQ(chunk[5], 'H') << "deps.png first chunk is not IHDR (byte 1)";
  EXPECT_EQ(chunk[6], 'D') << "deps.png first chunk is not IHDR (byte 2)";
  EXPECT_EQ(chunk[7], 'R') << "deps.png first chunk is not IHDR (byte 3)";

  // Now check graph.png
  chunk = graph_png + 8;

  // Get the chunk length (big-endian)
  length = (chunk[0] << 24) | (chunk[1] << 16) | (chunk[2] << 8) | chunk[3];

  // Check that the length is reasonable (IHDR is 13 bytes)
  EXPECT_EQ(length, 13u) << "graph.png IHDR chunk has unexpected length";

  // Check the chunk type (should be "IHDR")
  EXPECT_EQ(chunk[4], 'I') << "graph.png first chunk is not IHDR (byte 0)";
  EXPECT_EQ(chunk[5], 'H') << "graph.png first chunk is not IHDR (byte 1)";
  EXPECT_EQ(chunk[6], 'D') << "graph.png first chunk is not IHDR (byte 2)";
  EXPECT_EQ(chunk[7], 'R') << "graph.png first chunk is not IHDR (byte 3)";
}

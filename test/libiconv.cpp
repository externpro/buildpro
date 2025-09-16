#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <iconv.h>

class LibiconvTest : public ::testing::Test
{
protected:
  // Helper function to convert between encodings
  std::string convert(const std::string& input,
                      const char* from_encoding,
                      const char* to_encoding)
  {
    iconv_t cd = iconv_open(to_encoding, from_encoding);
    if (cd == (iconv_t)-1)
    {
      throw std::runtime_error("Failed to initialize iconv");
    }

    size_t inbytesleft = input.size();
    size_t outbytesleft = inbytesleft * 4; // Sufficiently large buffer
    std::vector<char> outbuf(outbytesleft);

    char* inptr = const_cast<char*>(input.data());
    char* outptr = outbuf.data();

    size_t result = iconv(cd, &inptr, &inbytesleft, &outptr, &outbytesleft);

    if (result == (size_t)-1)
    {
      iconv_close(cd);
      throw std::runtime_error("Conversion failed");
    }

    iconv_close(cd);

    return std::string(outbuf.data(), outptr - outbuf.data());
  }
};

TEST_F(LibiconvTest, TestUTF8ToUTF16Conversion)
{
  // Test string with various Unicode characters
  std::string utf8_str = u8"Hello, 世界! こんにちは! Привет!";

  // Convert to UTF-16
  std::string utf16_str = convert(utf8_str, "UTF-8", "UTF-16LE");

  // Convert back to UTF-8
  std::string roundtrip = convert(utf16_str, "UTF-16LE", "UTF-8");

  // The roundtrip should match the original
  EXPECT_EQ(utf8_str, roundtrip);
}

TEST_F(LibiconvTest, TestISO8859ToUTF8)
{
  // "Hällö Wörld!" in ISO-8859-1
  std::string iso8859_str = "H\xE4ll\xF6 W\xF6rld!";

  // Convert to UTF-8
  std::string utf8_str = convert(iso8859_str, "ISO-8859-1", "UTF-8");

  // The UTF-8 string should be longer due to multi-byte characters
  EXPECT_GT(utf8_str.length(), iso8859_str.length());

  // Convert back to ISO-8859-1
  std::string roundtrip = convert(utf8_str, "UTF-8", "ISO-8859-1//TRANSLIT");

  // The roundtrip should match the original
  EXPECT_EQ(iso8859_str, roundtrip);
}

TEST_F(LibiconvTest, TestInvalidInputHandling)
{
  // Invalid UTF-8 sequence (starts with 0x80 which is invalid in UTF-8)
  std::string invalid_utf8 = "\x80\x81\x82";

  // Should throw when trying to convert invalid input
  EXPECT_THROW(
    { convert(invalid_utf8, "UTF-8", "UTF-16LE"); }, std::runtime_error);
}

TEST_F(LibiconvTest, TestEmptyString)
{
  // Test with empty string
  std::string empty;
  std::string result = convert(empty, "UTF-8", "UTF-16LE");

  // Should return empty string
  EXPECT_TRUE(result.empty());
}

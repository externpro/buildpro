#include <string>

#include <curl/curl.h>
#include <gtest/gtest.h>

// Callback function to handle response data
static size_t WriteCallback(void* contents,
                            size_t size,
                            size_t nmemb,
                            void* userp)
{
  size_t real_size = size * nmemb;
  std::string* response = static_cast<std::string*>(userp);
  response->append(static_cast<char*>(contents), real_size);
  return real_size;
}

class CurlTest : public ::testing::Test
{
protected:
  CURL* curl;
  std::string response;
  char error_buffer[CURL_ERROR_SIZE];

  void SetUp() override
  {
    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Initialize a curl handle
    curl = curl_easy_init();
    ASSERT_NE(curl, nullptr) << "Failed to initialize CURL";

    // Set common options
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // Disable SSL verification for testing
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    // Set SSL options to handle potential compatibility issues
    curl_easy_setopt(
      curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2); // Force TLS 1.2
    curl_easy_setopt(curl,
                     CURLOPT_SSL_CIPHER_LIST,
                     "DEFAULT@SECLEVEL=1"); // Lower security level

    // Set timeout and other options
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L); // Set to 1L for debugging
  }

  void TearDown() override
  {
    // Cleanup
    if (curl)
    {
      curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
  }

  CURLcode PerformRequest(const std::string& url)
  {
    response.clear();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    return curl_easy_perform(curl);
  }
};

TEST_F(CurlTest, HttpGetRequest)
{
  // Test HTTP GET request to a public API
  CURLcode res = PerformRequest(
    "http://example.com"); // Using example.com as it's more stable

  // Check for errors
  if (res != CURLE_OK)
  {
    std::cout << "Warning: HTTP request failed: " << curl_easy_strerror(res)
              << ", " << error_buffer << std::endl;
    std::cout << "Skipping response content verification." << std::endl;
    GTEST_SKIP() << "Network request failed, skipping test";
    return;
  }

  // Check response
  EXPECT_FALSE(response.empty()) << "Empty response received";

  // Check for typical HTML content
  EXPECT_NE(response.find("<html"), std::string::npos)
    << "Response doesn't appear to be HTML";
}

TEST_F(CurlTest, HttpsRequest)
{
  // Test HTTPS request to a public API
  CURLcode res = PerformRequest(
    "https://example.com"); // Using example.com as it's more stable

  // Check for errors
  if (res != CURLE_OK)
  {
    std::cout << "Warning: HTTPS request failed: " << curl_easy_strerror(res)
              << ", " << error_buffer << std::endl;
    std::cout << "Skipping response content verification." << std::endl;
    GTEST_SKIP() << "Network request failed, skipping test";
    return;
  }

  // Check response
  EXPECT_FALSE(response.empty()) << "Empty response received";

  // Check for typical HTML content
  EXPECT_NE(response.find("<html"), std::string::npos)
    << "Response doesn't appear to be HTML";
}

TEST_F(CurlTest, VersionInfo)
{
  // Get libcurl version information
  curl_version_info_data* version_info = curl_version_info(CURLVERSION_NOW);

  ASSERT_NE(version_info, nullptr) << "Failed to get curl version info";

  // Print version information
  std::cout << "libcurl version: " << version_info->version << std::endl;
  std::cout << "SSL version: "
            << (version_info->ssl_version ? version_info->ssl_version : "none")
            << std::endl;

  // Check some basic version info
  EXPECT_NE(version_info->version, nullptr) << "Version string is null";
  EXPECT_GT(version_info->version_num, 0) << "Invalid version number";

  // Check that at least one protocol is supported
  bool has_http = false;
  const char* const* proto = version_info->protocols;
  while (*proto)
  {
    if (strcmp(*proto, "http") == 0 || strcmp(*proto, "https") == 0)
    {
      has_http = true;
      break;
    }
    ++proto;
  }
  EXPECT_TRUE(has_http) << "HTTP/HTTPS protocol not supported";
}

TEST_F(CurlTest, ErrorHandling)
{
  // Test with an invalid URL
  CURLcode res =
    PerformRequest("http://this-domain-should-not-exist-1234567890.org");

  // We expect this to fail
  EXPECT_NE(res, CURLE_OK) << "Expected request to fail but it succeeded";

  // Check that we got an error message
  EXPECT_FALSE(strlen(error_buffer) == 0) << "Expected an error message";

  std::cout << "Expected error (this is normal): " << error_buffer << std::endl;
}

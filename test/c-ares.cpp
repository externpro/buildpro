#include <cstring>
#include <string>
#include <vector>

#include <ares.h>
#include <gtest/gtest.h>

#ifdef _WIN32
#include <windows.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include "network_detection.h"

class CaresTest : public ::testing::Test
{
protected:
  ares_channel channel;

  void SetUp() override
  {
    // Skip all c-ares tests when offline since DNS resolution requires network
    if (!isNetworkAvailable())
    {
      GTEST_SKIP()
        << "Skipping c-ares tests - offline mode (DNS requires network)";
    }

#ifdef _WIN32
    // Initialize Winsock on Windows
    WSADATA wsaData;
    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    ASSERT_EQ(0, wsaResult) << "Failed to initialize Winsock";
#endif

    // Initialize c-ares library
    int status = ares_library_init(ARES_LIB_INIT_ALL);
    ASSERT_EQ(ARES_SUCCESS, status) << "Failed to initialize c-ares library";

    // Initialize a channel
    struct ares_options options;
    int optmask = ARES_OPT_FLAGS;
    options.flags = ARES_FLAG_NOCHECKRESP;
    options.servers = nullptr;
    options.nservers = 0;

    status = ares_init_options(&channel, &options, optmask);
    ASSERT_EQ(ARES_SUCCESS, status) << "Failed to initialize c-ares channel";
  }

  void TearDown() override
  {
    // Clean up
    ares_destroy(channel);
    ares_library_cleanup();

#ifdef _WIN32
    // Clean up Winsock on Windows
    WSACleanup();
#endif
  }

  static void callback(void* arg,
                       int status,
                       int timeouts,
                       struct hostent* host)
  {
    auto* result = reinterpret_cast<std::vector<std::string>*>(arg);

    if (status != ARES_SUCCESS)
    {
      result->push_back("DNS lookup failed: " +
                        std::string(ares_strerror(status)));
      return;
    }

    // Add the hostname
    result->push_back("Hostname: " + std::string(host->h_name));

    // Add all IP addresses
    char ip[INET6_ADDRSTRLEN];
    for (int i = 0; host->h_addr_list[i] != nullptr; ++i)
    {
      if (host->h_addrtype == AF_INET)
      {
#ifdef _WIN32
        // Windows has different function names for IP conversion
        struct in_addr addr;
        memcpy(&addr, host->h_addr_list[i], sizeof(struct in_addr));
        InetNtopA(AF_INET, &addr, ip, sizeof(ip));
#else
        inet_ntop(AF_INET, host->h_addr_list[i], ip, sizeof(ip));
#endif
        result->push_back("IPv4: " + std::string(ip));
      }
      else if (host->h_addrtype == AF_INET6)
      {
#ifdef _WIN32
        struct in6_addr addr;
        memcpy(&addr, host->h_addr_list[i], sizeof(struct in6_addr));
        InetNtopA(AF_INET6, &addr, ip, sizeof(ip));
#else
        inet_ntop(AF_INET6, host->h_addr_list[i], ip, sizeof(ip));
#endif
        result->push_back("IPv6: " + std::string(ip));
      }
    }
  }

  std::vector<std::string> resolveHost(const std::string& hostname)
  {
    std::vector<std::string> result;

    // Start the query
    ares_gethostbyname(channel, hostname.c_str(), AF_INET, callback, &result);

    // Process the query
    int nfds, count;
    fd_set readers, writers;
    timeval tv, *tvp;

    while (true)
    {
      FD_ZERO(&readers);
      FD_ZERO(&writers);

      nfds = ares_fds(channel, &readers, &writers);
      if (nfds == 0)
      {
        break; // No more file descriptors to wait for
      }

      tvp = ares_timeout(channel, NULL, &tv);

      count = select(nfds, &readers, &writers, NULL, tvp);
      if (count < 0)
      {
#ifdef _WIN32
        int error = WSAGetLastError();
        if (error != WSAEINVAL)
        {
          char errorMsg[256];
          FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
                         NULL,
                         error,
                         0,
                         errorMsg,
                         sizeof(errorMsg),
                         NULL);
          result.push_back("select() failed: " + std::string(errorMsg));
          break;
        }
#else
        if (errno != EINVAL)
        {
          result.push_back("select() failed: " + std::string(strerror(errno)));
          break;
        }
#endif
      }

      ares_process(channel, &readers, &writers);
    }

    return result;
  }
};

TEST_F(CaresTest, BasicResolution)
{
  // Test resolving a well-known domain
  auto results = resolveHost("example.com");

  // We should have at least the hostname and one IP address
  EXPECT_GE(results.size(), 2)
    << "Expected at least 2 results (hostname and IP)";

  // The first result should be the hostname
  EXPECT_NE(results[0].find("Hostname: "), std::string::npos)
    << "First result should be the hostname";

  // Check if we have at least one IP address
  bool hasIp = false;
  for (const auto& result : results)
  {
    if (result.find("IPv4: ") == 0 || result.find("IPv6: ") == 0)
    {
      hasIp = true;
      break;
    }
  }
  EXPECT_TRUE(hasIp) << "No IP addresses were returned";
}

TEST_F(CaresTest, LibraryVersion)
{
  // Test that we can get the library version
  const char* version = ares_version(NULL);
  EXPECT_NE(version, nullptr) << "Failed to get c-ares version";

  if (version)
  {
    std::cout << "Using c-ares version: " << version << std::endl;
  }
}

TEST_F(CaresTest, InvalidHostname)
{
  // Test resolving an invalid hostname
  auto results = resolveHost("this-should-not-exist-1234567890.example.com");

  // We should have an error message
  EXPECT_FALSE(results.empty())
    << "Expected an error message for invalid hostname";

  if (!results.empty())
  {
    EXPECT_NE(results[0].find("DNS lookup failed"), std::string::npos)
      << "Expected DNS lookup failure for invalid hostname";
  }
}

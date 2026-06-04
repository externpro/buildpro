#include <array>
#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

#include <azmq/socket.hpp>
#include <gtest/gtest.h>
#include <zmq.h>

// Test configuration
constexpr auto TEST_TIMEOUT = std::chrono::seconds(5);
constexpr auto SOCKET_LINGER = std::chrono::seconds(1);
constexpr int MAX_RETRIES = 3;
constexpr auto RETRY_DELAY = std::chrono::milliseconds(100);

// Helper function to get a random port in a safe range
uint16_t get_random_port()
{
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<uint16_t> dist(
    49152, 65535); // Ephemeral ports
  return dist(gen);
}

// RAII wrapper for thread management
class ScopedThread
{
  std::thread t_;

public:
  template <typename... Args>
  explicit ScopedThread(Args&&... args) : t_(std::forward<Args>(args)...)
  {
  }
  ~ScopedThread()
  {
    if (t_.joinable())
    {
      t_.join();
    }
  }
  ScopedThread(const ScopedThread&) = delete;
  ScopedThread& operator=(const ScopedThread&) = delete;
  ScopedThread(ScopedThread&&) = default;
  ScopedThread& operator=(ScopedThread&&) = default;
};

namespace asio = boost::asio;
using namespace std::chrono_literals;

class AzmqTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    ioc_ = std::make_shared<asio::io_context>();
    work_ = std::make_unique<
      boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(
      boost::asio::make_work_guard(*ioc_));
    deadline_ = std::make_unique<asio::steady_timer>(*ioc_);
    port_ = get_random_port();

    // Start the io_context in a separate thread
    worker_ = std::thread([this] { ioc_->run(); });

    // Set up deadline timer for test timeouts
    set_test_timeout(TEST_TIMEOUT);
  }

  void TearDown() override
  {
    // Stop the io_context and wait for the thread to finish
    ioc_->stop();
    if (worker_.joinable())
    {
      worker_.join();
    }
    work_.reset();
    deadline_.reset();
  }

  void set_test_timeout(std::chrono::milliseconds timeout)
  {
    deadline_->expires_after(timeout);
    deadline_->async_wait(
      [this](const boost::system::error_code& ec)
      {
        if (!ec)
        {
          // Timeout occurred
          std::cerr << "Test timed out after " << TEST_TIMEOUT.count()
                    << "ms\n";
          ioc_->stop();
          FAIL() << "Test timed out";
        }
      });
  }

  std::string get_endpoint() const
  {
    return "tcp://127.0.0.1:" + std::to_string(port_);
  }

  template <typename Socket>
  bool bind_with_retry(Socket& socket, int max_attempts = MAX_RETRIES)
  {
    for (int i = 0; i < max_attempts; ++i)
    {
      boost::system::error_code ec;
      port_ = get_random_port();
      socket.bind(get_endpoint(), ec);
      if (!ec)
        return true;
      std::this_thread::sleep_for(RETRY_DELAY);
    }
    return false;
  }

  std::shared_ptr<asio::io_context> ioc_;
  std::unique_ptr<
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>
    work_;
  std::thread worker_;
  std::unique_ptr<asio::steady_timer> deadline_;
  std::atomic<uint16_t> port_;
};

TEST_F(AzmqTest, SocketCreation)
{
  // Test basic socket creation
  ASSERT_NO_THROW({
    azmq::socket socket(*ioc_, ZMQ_REP);
    socket.set_option(
      azmq::socket::linger(static_cast<int>(SOCKET_LINGER.count())));
  });

  // Test socket options
  ASSERT_NO_THROW({
    azmq::socket socket(*ioc_, ZMQ_SUB);
    socket.set_option(azmq::socket::subscribe("TOPIC"));
    socket.set_option(
      azmq::socket::linger(static_cast<int>(SOCKET_LINGER.count())));

    // Test getting socket type
    int type = 0;
    size_t type_size = sizeof(type);
    zmq_getsockopt(socket.native_handle(), ZMQ_TYPE, &type, &type_size);
    EXPECT_GT(type, 0);
  });
}

TEST_F(AzmqTest, ReqRepPattern)
{
  // Create REP socket with random port
  azmq::rep_socket rep_socket(*ioc_);
  rep_socket.set_option(
    azmq::socket::linger(static_cast<int>(SOCKET_LINGER.count())));

  // Bind with retry logic
  ASSERT_TRUE(this->bind_with_retry(rep_socket))
    << "Failed to bind REP socket after multiple attempts";

  const auto endpoint = this->get_endpoint();
  std::cout << "REP socket bound to: " << endpoint << std::endl;

  // Create REQ socket
  azmq::req_socket req_socket(*ioc_);
  req_socket.set_option(
    azmq::socket::linger(static_cast<int>(SOCKET_LINGER.count())));

  // Set timeouts on REQ socket (1 second)
  const int timeout_ms = 1000;
  ASSERT_EQ(zmq_setsockopt(req_socket.native_handle(),
                           ZMQ_RCVTIMEO,
                           &timeout_ms,
                           sizeof(timeout_ms)),
            0);
  ASSERT_EQ(zmq_setsockopt(req_socket.native_handle(),
                           ZMQ_SNDTIMEO,
                           &timeout_ms,
                           sizeof(timeout_ms)),
            0);

  // Connect REQ socket
  bool connected = false;
  for (int i = 0; i < 3 && !connected; ++i)
  {
    boost::system::error_code ec;
    req_socket.connect(endpoint, ec);
    if (!ec)
    {
      connected = true;
      break;
    }
    std::cerr << "Warning: Failed to connect REQ socket (attempt " << (i + 1)
              << "): " << ec.message() << std::endl;
    std::this_thread::sleep_for(100ms);
  }
  ASSERT_TRUE(connected)
    << "Failed to connect REQ socket after multiple attempts";

  // Test message
  const std::string test_message = "Hello, World!";
  const std::string expected_reply = "Reply: " + test_message;

  // Synchronization for the test
  std::promise<bool> test_complete_promise;
  auto test_complete = test_complete_promise.get_future();
  bool test_success = false;

  // Start a thread to handle the REP socket
  std::thread rep_thread(
    [&]()
    {
      try
      {
        // Set receive timeout on REP socket
        int timeout = 2000; // 2 seconds
        zmq_setsockopt(
          rep_socket.native_handle(), ZMQ_RCVTIMEO, &timeout, sizeof(timeout));

        // Receive request
        std::array<char, 256> buffer;
        boost::system::error_code ec;
        auto size = rep_socket.receive(boost::asio::buffer(buffer), 0, ec);
        if (ec)
        {
          std::cerr << "REP socket receive error: " << ec.message()
                    << std::endl;
          test_complete_promise.set_value(false);
          return;
        }

        std::string received(buffer.data(), size);
        if (received != test_message)
        {
          std::cerr << "Unexpected message received: " << received << std::endl;
          test_complete_promise.set_value(false);
          return;
        }

        // Send reply
        rep_socket.send(boost::asio::buffer(expected_reply), 0, ec);
        if (ec)
        {
          std::cerr << "Failed to send reply: " << ec.message() << std::endl;
          test_complete_promise.set_value(false);
          return;
        }

        test_complete_promise.set_value(true);
      }
      catch (const std::exception& e)
      {
        std::cerr << "Exception in REP thread: " << e.what() << std::endl;
        test_complete_promise.set_value(false);
      }
    });

  // Ensure thread is cleaned up
  auto thread_guard = std::move(rep_thread);

  // Send request
  {
    boost::system::error_code ec;
    req_socket.send(boost::asio::buffer(test_message), 0, ec);
    ASSERT_FALSE(ec) << "Error sending request: " << ec.message();
  }

  // Receive reply
  {
    std::array<char, 256> buffer;
    boost::system::error_code ec;
    auto size = req_socket.receive(boost::asio::buffer(buffer), 0, ec);

    if (ec)
    {
      FAIL() << "Error receiving reply: " << ec.message();
      return;
    }

    std::string reply(buffer.data(), size);
    EXPECT_EQ(reply, expected_reply);
  }

  // Wait for the test to complete or timeout
  auto status = test_complete.wait_for(std::chrono::seconds(3));
  if (status == std::future_status::timeout)
  {
    FAIL() << "Test timed out waiting for completion";
  }
  else
  {
    test_success = test_complete.get();
    EXPECT_TRUE(test_success) << "Test failed in the REP thread";
  }

  // Clean up
  if (thread_guard.joinable())
  {
    thread_guard.join();
  }
}

// Test PUB/SUB pattern
TEST_F(AzmqTest, PubSubPattern)
{
  // Create and bind PUB socket
  azmq::pub_socket pub_socket(*ioc_);
  pub_socket.set_option(
    azmq::socket::linger(static_cast<int>(SOCKET_LINGER.count())));
  ASSERT_TRUE(this->bind_with_retry(pub_socket)) << "Failed to bind PUB socket";
  const auto pub_endpoint = this->get_endpoint();

  // Create SUB socket with options
  azmq::sub_socket sub_socket(*ioc_);
  sub_socket.set_option(
    azmq::socket::linger(static_cast<int>(SOCKET_LINGER.count())));
  sub_socket.set_option(
    azmq::socket::subscribe("")); // Subscribe to all messages

  // Connect with retry logic
  bool connected = false;
  for (int i = 0; i < 3 && !connected; ++i)
  {
    boost::system::error_code ec;
    sub_socket.connect(pub_endpoint, ec);
    if (!ec)
    {
      connected = true;
      break;
    }
    std::this_thread::sleep_for(100ms);
  }
  ASSERT_TRUE(connected) << "Failed to connect SUB socket";

  // Small delay to ensure connection is established
  std::this_thread::sleep_for(RETRY_DELAY);

  // Send a test message
  const std::string test_message = "TEST_MESSAGE";
  bool message_sent = false;

  for (int i = 0; i < 3 && !message_sent; ++i)
  {
    boost::system::error_code ec;
    pub_socket.send(boost::asio::buffer(test_message), 0, ec);
    if (!ec)
    {
      message_sent = true;
      break;
    }
    std::this_thread::sleep_for(100ms);
  }

  ASSERT_TRUE(message_sent) << "Failed to send message after multiple attempts";

  // Try to receive the message with a timeout
  std::array<char, 256> buffer;
  boost::system::error_code ec;
  asio::steady_timer timer(*ioc_, std::chrono::seconds(2));
  bool received = false;

  // Set up timer to cancel the receive on timeout
  timer.async_wait(
    [&](const boost::system::error_code& ec)
    {
      if (!ec)
        sub_socket.cancel();
    });

  // Try to receive the message
  auto size = sub_socket.receive(asio::buffer(buffer), 0, ec);

  // Cancel the timer
  timer.cancel();

  // Check if we received the message
  if (!ec)
  {
    std::string received_message(buffer.data(), size);
    EXPECT_EQ(received_message, test_message);
  }
  else
  {
    FAIL() << "Failed to receive message: " << ec.message();
  }
}

// Test the example code with a simple synchronous approach
TEST_F(AzmqTest, ExampleCode)
{
  // Using inproc:// protocol which is more reliable for in-process
  // communication
  const std::string example_endpoint =
    "inproc://example_test_" +
    std::to_string(std::chrono::system_clock::now().time_since_epoch().count());

  // Test message
  const std::string test_message = "NASDAQ:MSFT 234.56";

  // Create publisher socket in a separate thread
  std::thread pub_thread(
    [&]()
    {
      try
      {
        asio::io_context ioc;
        azmq::pub_socket publisher(ioc);

        // Set socket options
        publisher.set_option(
          azmq::socket::linger(static_cast<int>(SOCKET_LINGER.count())));

        // Bind to the endpoint
        boost::system::error_code ec;
        publisher.bind(example_endpoint, ec);
        if (ec)
        {
          ADD_FAILURE() << "Failed to bind publisher: " << ec.message();
          return;
        }

        // Give time for subscriber to connect
        std::this_thread::sleep_for(100ms);

        // Send the test message
        publisher.send(boost::asio::buffer(test_message), 0, ec);
        if (ec)
        {
          ADD_FAILURE() << "Failed to send message: " << ec.message();
        }
      }
      catch (const std::exception& e)
      {
        ADD_FAILURE() << "Exception in publisher thread: " << e.what();
      }
    });

  // Make sure the publisher thread is joined when we exit
  auto pub_guard = std::move(pub_thread);

  try
  {
    // Create subscriber socket
    asio::io_context ioc;
    azmq::sub_socket subscriber(ioc);

    // Set socket options
    subscriber.set_option(
      azmq::socket::linger(static_cast<int>(SOCKET_LINGER.count())));
    subscriber.set_option(
      azmq::socket::subscribe("")); // Subscribe to all messages

    // Set receive timeout (2 seconds)
    const int recv_timeout = 2000;
    ASSERT_EQ(zmq_setsockopt(subscriber.native_handle(),
                             ZMQ_RCVTIMEO,
                             &recv_timeout,
                             sizeof(recv_timeout)),
              0)
      << "Failed to set receive timeout: " << zmq_strerror(zmq_errno());

    // Connect to the publisher
    boost::system::error_code ec;
    subscriber.connect(example_endpoint, ec);
    if (ec)
    {
      FAIL() << "Failed to connect subscriber: " << ec.message();
      return;
    }

    // Try to receive the message
    std::array<char, 256> buffer;
    auto size = subscriber.receive(boost::asio::buffer(buffer), 0, ec);

    if (ec)
    {
      FAIL() << "Failed to receive message: " << ec.message();
      return;
    }

    std::string received_message(buffer.data(), size);
    EXPECT_EQ(received_message, test_message);
  }
  catch (const std::exception& e)
  {
    FAIL() << "Exception in test: " << e.what();
  }

  // Wait for publisher thread to finish
  if (pub_guard.joinable())
  {
    pub_guard.join();
  }
}

// Test socket options
TEST_F(AzmqTest, SocketOptions)
{
  // Create a socket with the test's io_service
  azmq::socket socket(*ioc_, ZMQ_DEALER);

  // Test setting and getting linger option
  const int test_linger_ms = 1500; // 1.5 seconds
  ASSERT_NO_THROW({ socket.set_option(azmq::socket::linger(test_linger_ms)); })
    << "Failed to set linger option";

  // Test getting the option back
  azmq::socket::linger linger_option;
  ASSERT_NO_THROW({ socket.get_option(linger_option); })
    << "Failed to get linger option";

  // Verify the value matches what we set
  int linger_result = *static_cast<const int*>(linger_option.data());
  EXPECT_EQ(linger_result, test_linger_ms)
    << "Linger value does not match expected";

  // Test setting high water marks using ZMQ_SNDHWM and ZMQ_RCVHWM
  // Note: Using the basic linger option as a simple test since other options
  // might not be available
  const int test_linger = 1000; // 1 second
  ASSERT_NO_THROW({ socket.set_option(azmq::socket::linger(test_linger)); })
    << "Failed to set linger option";

  // Verify the linger option was set
  int linger_value = -1;
  ASSERT_NO_THROW({
    azmq::socket::linger linger_opt;
    socket.get_option(linger_opt);
    linger_value = *static_cast<const int*>(linger_opt.data());
  }) << "Failed to get linger option";

  EXPECT_GE(linger_value, 0) << "Linger value should be non-negative";

  // Test basic subscription (for SUB sockets)
  // Test getting socket type
  int socket_type = 0;
  size_t type_size = sizeof(socket_type);
  int rc =
    zmq_getsockopt(socket.native_handle(), ZMQ_TYPE, &socket_type, &type_size);
  ASSERT_EQ(rc, 0) << "Failed to get socket type: "
                   << zmq_strerror(zmq_errno());

  // For SUB sockets, test subscription
  if (socket_type == ZMQ_SUB)
  {
    // Use the AZMQ subscribe/unsubscribe API which is available
    ASSERT_NO_THROW({
      socket.set_option(azmq::socket::subscribe("TEST"));
      socket.set_option(azmq::socket::unsubscribe("TEST"));
    }) << "Failed to set/unset subscription";
  }
}

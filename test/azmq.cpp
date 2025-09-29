#include <array>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <boost/asio.hpp>

#include <azmq/socket.hpp>
#include <gtest/gtest.h>

namespace asio = boost::asio;
using namespace std::chrono_literals;

class AzmqTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Create a new io_service for each test
    ios = std::make_shared<asio::io_service>();
  }

  void TearDown() override
  {
    // Clean up io_service
    if (ios)
    {
      ios->stop();
    }
  }

  std::shared_ptr<asio::io_service> ios;
};

// Test basic socket creation and option setting
TEST_F(AzmqTest, SocketCreation)
{
  // Create a socket
  ASSERT_NO_THROW({ azmq::socket socket(*ios, ZMQ_REP); });

  // Create a socket and set options
  ASSERT_NO_THROW({
    azmq::socket socket(*ios, ZMQ_SUB);
    socket.set_option(azmq::socket::subscribe("TOPIC"));
  });
}

// Test basic REQ/REP pattern
TEST_F(AzmqTest, ReqRepPattern)
{
  // Create REP socket
  azmq::rep_socket rep_socket(*ios);
  ASSERT_NO_THROW({ rep_socket.bind("tcp://127.0.0.1:5555"); });

  // Create REQ socket
  azmq::req_socket req_socket(*ios);
  ASSERT_NO_THROW({ req_socket.connect("tcp://127.0.0.1:5555"); });

  // Create a thread for the REP socket
  std::thread rep_thread(
    [&]()
    {
      try
      {
        std::array<char, 256> buffer;
        boost::system::error_code ec;

        // Receive the request
        std::size_t size = rep_socket.receive(asio::buffer(buffer), 0, ec);
        ASSERT_FALSE(ec) << "Error receiving message: " << ec.message();

        std::string received(buffer.data(), size);
        EXPECT_EQ(received, "Hello");

        // Send the reply
        std::string reply = "World";
        rep_socket.send(asio::buffer(reply), 0, ec);
        ASSERT_FALSE(ec) << "Error sending reply: " << ec.message();
      }
      catch (const std::exception& e)
      {
        ADD_FAILURE() << "Exception in REP thread: " << e.what();
      }
    });

  // Send a request from the REQ socket
  std::string request = "Hello";
  boost::system::error_code ec;
  req_socket.send(asio::buffer(request), 0, ec);
  ASSERT_FALSE(ec) << "Error sending request: " << ec.message();

  // Receive the reply
  std::array<char, 256> buffer;
  std::size_t size = req_socket.receive(asio::buffer(buffer), 0, ec);
  ASSERT_FALSE(ec) << "Error receiving reply: " << ec.message();

  std::string reply(buffer.data(), size);
  EXPECT_EQ(reply, "World");

  // Clean up
  if (rep_thread.joinable())
  {
    rep_thread.join();
  }
}

// Test PUB/SUB pattern
TEST_F(AzmqTest, PubSubPattern)
{
  // Create PUB socket
  azmq::pub_socket pub_socket(*ios);
  ASSERT_NO_THROW({ pub_socket.bind("tcp://127.0.0.1:5556"); });

  // Create SUB socket
  azmq::sub_socket sub_socket(*ios);
  ASSERT_NO_THROW({
    sub_socket.connect("tcp://127.0.0.1:5556");
    sub_socket.set_option(azmq::socket::subscribe("NASDAQ"));
  });

  // Give time for the connection to establish
  std::this_thread::sleep_for(100ms);

  // Create a thread for the SUB socket
  std::atomic<bool> message_received{false};
  std::thread sub_thread(
    [&]()
    {
      try
      {
        std::array<char, 256> buffer;
        boost::system::error_code ec;

        // Try to receive a message with timeout
        for (int i = 0; i < 5 && !message_received; ++i)
        {
          try
          {
            std::size_t size = sub_socket.receive(asio::buffer(buffer), 0);
            {
              std::string received(buffer.data(), size);
              EXPECT_EQ(received, "NASDAQ:AAPL 123.45");
              message_received = true;
              break;
            }
          }
          catch (const std::exception& e)
          {
            std::cerr << "Error in receive attempt " << i << ": " << e.what()
                      << std::endl;
          }
          std::this_thread::sleep_for(100ms);
        }
      }
      catch (const std::exception& e)
      {
        ADD_FAILURE() << "Exception in SUB thread: " << e.what();
      }
    });

  // Send a few messages from the PUB socket to ensure delivery
  for (int i = 0; i < 3; ++i)
  {
    std::string message = "NASDAQ:AAPL 123.45";
    boost::system::error_code ec;
    pub_socket.send(asio::buffer(message), 0);
    std::this_thread::sleep_for(100ms);
  }

  // Wait for the subscriber thread
  if (sub_thread.joinable())
  {
    sub_thread.join();
  }

  EXPECT_TRUE(message_received) << "No message was received by the subscriber";
}

// Test the example code from the user request
TEST_F(AzmqTest, ExampleCode)
{
  // This is a simplified version of the example that doesn't require external
  // connections but demonstrates the same API usage

  // Create a local endpoint for testing
  std::string endpoint = "inproc://example_test";

  // Create publisher thread
  std::thread pub_thread(
    [&]()
    {
      try
      {
        asio::io_service ios;
        azmq::pub_socket publisher(ios);
        publisher.bind(endpoint);

        // Send a few test messages
        for (int i = 0; i < 3; ++i)
        {
          std::string message = "NASDAQ:MSFT 234.56";
          boost::system::error_code ec;
          publisher.send(asio::buffer(message), 0);
          std::this_thread::sleep_for(100ms);
        }
      }
      catch (const std::exception& e)
      {
        ADD_FAILURE() << "Exception in publisher thread: " << e.what();
      }
    });

  // Give publisher time to bind
  std::this_thread::sleep_for(100ms);

  // Create subscriber
  asio::io_service sub_ios;
  azmq::sub_socket subscriber(sub_ios);
  subscriber.connect(endpoint);
  subscriber.set_option(azmq::socket::subscribe("NASDAQ"));

  // Create a buffer and receive messages
  std::array<char, 256> buf;
  bool received = false;

  // Try to receive a message
  for (int i = 0; i < 5 && !received; ++i)
  {
    boost::system::error_code ec;
    try
    {
      auto size = subscriber.receive(asio::buffer(buf), 0);
      {
        std::string message(buf.data(), size);
        EXPECT_EQ(message, "NASDAQ:MSFT 234.56");
        received = true;
        break;
      }
    }
    catch (const std::exception& e)
    {
      std::cerr << "Error in receive attempt " << i << ": " << e.what()
                << std::endl;
    }
    std::this_thread::sleep_for(100ms);
  }

  // Clean up
  if (pub_thread.joinable())
  {
    pub_thread.join();
  }

  EXPECT_TRUE(received) << "No message was received in the example test";
}

// Test socket options
TEST_F(AzmqTest, SocketOptions)
{
  azmq::socket socket(*ios, ZMQ_DEALER);

  // Test setting linger option
  int linger_value = 100;
  ASSERT_NO_THROW({ socket.set_option(azmq::socket::linger(linger_value)); });

  azmq::socket::linger linger_option;
  ASSERT_NO_THROW({ socket.get_option(linger_option); });

  int linger_result = *static_cast<const int*>(linger_option.data());
  EXPECT_EQ(linger_result, linger_value);
}

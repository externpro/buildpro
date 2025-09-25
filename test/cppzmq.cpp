#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>

#include <gtest/gtest.h>
#include <zmq.hpp>

using namespace std::chrono_literals;

class CppZmqTest : public ::testing::Test
{
protected:
  zmq::context_t context;
  std::string endpoint = "tcp://127.0.0.1:5555";

  void SetUp() override
  {
    // Initialize ZMQ context with a single I/O thread
    context = zmq::context_t(1);
  }

  void TearDown() override
  {
    // Close the ZMQ context
    context.close();
  }
};

TEST_F(CppZmqTest, BasicSendReceive)
{
  // Setup server socket
  zmq::socket_t server(context, ZMQ_REP);
  server.bind(endpoint);

  // Setup client socket in a separate thread
  std::thread client_thread(
    [this]()
    {
      zmq::socket_t client(context, ZMQ_REQ);
      client.connect(endpoint);

      // Send a message
      std::string message = "Hello";
      zmq::message_t request(message.begin(), message.end());
      client.send(request, zmq::send_flags::none);

      // Receive reply
      zmq::message_t reply;
      auto recv_result = client.recv(reply, zmq::recv_flags::none);
      std::string reply_str(static_cast<char*>(reply.data()), reply.size());
      EXPECT_EQ(reply_str, "World");
    });

  // Server receives the message
  zmq::message_t request;
  auto recv_result = server.recv(request, zmq::recv_flags::none);
  std::string request_str(static_cast<char*>(request.data()), request.size());
  EXPECT_EQ(request_str, "Hello");

  // Server sends a reply
  std::string reply_msg = "World";
  zmq::message_t reply(reply_msg.begin(), reply_msg.end());
  server.send(reply, zmq::send_flags::none);

  // Clean up
  if (client_thread.joinable())
  {
    client_thread.join();
  }
}

TEST_F(CppZmqTest, PubSubPattern)
{
  std::atomic<bool> received{false};
  std::atomic<bool> publisher_ready{false};
  std::atomic<bool> publisher_done{false};
  std::string topic = "test_topic";
  std::string message = "Test message";
  const int max_attempts = 5; // Increased from 3 to 5
  const int port = 5556;
  std::string endpoint = "tcp://127.0.0.1:" + std::to_string(port);
  std::mutex cout_mutex;

  // Setup publisher in a separate thread
  std::thread publisher_thread(
    [&]()
    {
      try
      {
        zmq::socket_t publisher(context, ZMQ_PUB);
        // Set high water mark to prevent publisher from dropping messages
        const int hwm = 1000;
        publisher.set(zmq::sockopt::sndhwm, hwm);

        // Bind to the endpoint
        publisher.bind(endpoint);
        publisher_ready = true;

        // Give subscribers time to connect
        std::this_thread::sleep_for(500ms);

        // Publish multiple messages to ensure delivery
        for (int i = 0; i < max_attempts && !publisher_done; ++i)
        {
          try
          {
            // Send topic
            if (!publisher.send(zmq::buffer(topic), zmq::send_flags::sndmore))
            {
              std::lock_guard<std::mutex> lock(cout_mutex);
              std::cerr << "Failed to send topic on attempt " << (i + 1)
                        << std::endl;
              continue;
            }

            // Send message
            zmq::message_t msg(message.begin(), message.end());
            if (!publisher.send(msg, zmq::send_flags::none))
            {
              std::lock_guard<std::mutex> lock(cout_mutex);
              std::cerr << "Failed to send message on attempt " << (i + 1)
                        << std::endl;
              continue;
            }

            std::this_thread::sleep_for(100ms); // Slight delay between messages
          }
          catch (const zmq::error_t& e)
          {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cerr << "Publisher send error on attempt " << (i + 1) << ": "
                      << e.what() << " (" << e.num() << ")" << std::endl;
            if (e.num() == ETERM)
              break; // Context was terminated
          }
        }
      }
      catch (const std::exception& e)
      {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cerr << "Publisher thread error: " << e.what() << std::endl;
      }
    });

  // Wait for publisher to be ready
  int wait_count = 0;
  while (!publisher_ready && wait_count++ < 10)
  {
    std::this_thread::sleep_for(100ms);
  }

  if (!publisher_ready)
  {
    FAIL() << "Publisher thread failed to initialize";
    if (publisher_thread.joinable())
    {
      publisher_thread.join();
    }
    return;
  }

  // Setup subscriber
  try
  {
    zmq::socket_t subscriber(context, ZMQ_SUB);
    // Set high water mark and timeout
    subscriber.set(zmq::sockopt::rcvhwm, 1000);
    subscriber.set(zmq::sockopt::rcvtimeo, 500); // 500ms timeout per recv

    // Connect and subscribe
    subscriber.connect(endpoint);
    subscriber.set(zmq::sockopt::subscribe, topic);

    // Set linger to 0 for immediate shutdown
    subscriber.set(zmq::sockopt::linger, 0);

    // Wait for connection to establish
    std::this_thread::sleep_for(200ms);

    bool message_received = false;
    for (int attempt = 0; attempt < max_attempts && !message_received;
         ++attempt)
    {
      try
      {
        zmq::message_t topic_msg;
        zmq::message_t content;

        // Try to receive the topic with a timeout
        if (subscriber.recv(topic_msg))
        {
          std::string received_topic(topic_msg.to_string());
          EXPECT_EQ(received_topic, topic) << "Unexpected topic received";

          // Try to receive the message content
          if (subscriber.recv(content))
          {
            std::string received_content(content.to_string());
            EXPECT_EQ(received_content, message)
              << "Unexpected message content";
            message_received = true;
            received = true;
            break;
          }
          else
          {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cerr << "Failed to receive message content on attempt "
                      << (attempt + 1) << std::endl;
          }
        }
        else
        {
          std::lock_guard<std::mutex> lock(cout_mutex);
          std::cerr << "Failed to receive topic on attempt " << (attempt + 1)
                    << std::endl;
        }
      }
      catch (const zmq::error_t& e)
      {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cerr << "Subscriber error on attempt " << (attempt + 1) << ": "
                  << e.what() << " (" << e.num() << ")" << std::endl;
        if (e.num() == ETERM)
          break; // Context was terminated
      }

      // Small delay before next attempt
      std::this_thread::sleep_for(100ms);
    }

    // Clean up subscriber
    subscriber.close();
  }
  catch (const std::exception& e)
  {
    FAIL() << "Subscriber error: " << e.what();
  }

  // Signal publisher to stop
  publisher_done = true;

  // Clean up publisher thread
  if (publisher_thread.joinable())
  {
    publisher_thread.join();
  }

  EXPECT_TRUE(received) << "No message received after " << max_attempts
                        << " attempts";
}

TEST_F(CppZmqTest, VersionCheck)
{
  int major, minor, patch;
  zmq_version(&major, &minor, &patch);

  std::cout << "Using ZeroMQ version: " << major << "." << minor << "." << patch
            << std::endl;

  // Just check that we got some version numbers
  EXPECT_GE(major, 0);
  EXPECT_GE(minor, 0);
  EXPECT_GE(patch, 0);
}

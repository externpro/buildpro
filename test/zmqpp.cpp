#include <string>

#include <gtest/gtest.h>
#include <zmqpp/zmqpp.hpp>

using namespace zmqpp;

class ZmqppTest : public ::testing::Test
{
protected:
  zmqpp::context context;
  std::string endpoint = "inproc://test_zmqpp";
};

TEST_F(ZmqppTest, BasicRequestReply)
{
  // Create and bind a REP socket
  socket rep_socket(context, socket_type::rep);
  rep_socket.bind(endpoint);

  // Create and connect a REQ socket
  socket req_socket(context, socket_type::req);
  req_socket.connect(endpoint);

  // Send a message
  std::string test_message = "Hello, ZMQPP!";
  message request;
  request << test_message;
  ASSERT_TRUE(req_socket.send(request, true));

  // Receive the message
  message received;
  ASSERT_TRUE(rep_socket.receive(received, true));

  // Verify the message
  std::string received_text;
  received >> received_text;
  EXPECT_EQ(test_message, received_text);

  // Send a reply
  message reply;
  reply << ("Reply: " + received_text);
  ASSERT_TRUE(rep_socket.send(reply, true));

  // Receive the reply
  message reply_received;
  ASSERT_TRUE(req_socket.receive(reply_received, true));

  std::string reply_text;
  reply_received >> reply_text;
  EXPECT_EQ("Reply: " + test_message, reply_text);
}

TEST_F(ZmqppTest, ContextAndSocketOptions)
{
  // Test context options
  EXPECT_GE(context.get(context_option::io_threads), 1);

  // Test socket options
  socket s(context, socket_type::req);
  std::string endpoint = "inproc://test_socket";
  s.bind(endpoint);

  // Test getting last endpoint
  std::string last_endpoint;
  s.get(socket_option::last_endpoint, last_endpoint);
  EXPECT_FALSE(last_endpoint.empty());

  // Test setting and getting socket options
  int send_timeout = 1000;
  s.set(socket_option::send_timeout, send_timeout);

  int retrieved_timeout = 0;
  s.get(socket_option::send_timeout, retrieved_timeout);
  EXPECT_EQ(send_timeout, retrieved_timeout);
}

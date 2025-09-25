#include <chrono>
#include <string>
#include <thread>

#include <gtest/gtest.h>
#include <sodium.h>
#include <zmq.h>

class ZMQTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Initialize libsodium for curve key generation
    if (sodium_init() < 0)
    {
      FAIL() << "Failed to initialize libsodium";
    }

    // Initialize ZMQ context
    ctx = zmq_ctx_new();
    if (!ctx)
    {
      FAIL() << "Failed to create ZMQ context: " << zmq_strerror(zmq_errno());
    }
  }

  void TearDown() override
  {
    if (ctx)
    {
      zmq_ctx_term(ctx);
    }
  }

  // Generate Curve keypair using libsodium
  std::pair<std::string, std::string> generate_curve_keypair()
  {
    unsigned char public_key[32];
    unsigned char secret_key[32];
    crypto_box_keypair(public_key, secret_key);

    // Convert binary keys to Z85 format (40 chars + null terminator)
    char public_z85[41];
    char secret_z85[41];
    zmq_z85_encode(public_z85, public_key, 32);
    zmq_z85_encode(secret_z85, secret_key, 32);

    return {public_z85, secret_z85};
  }

  void* ctx = nullptr;
};

// Test basic REQ/REP pattern
TEST_F(ZMQTest, BasicReqRep)
{
  // Create and bind REP socket
  void* rep_socket = zmq_socket(ctx, ZMQ_REP);
  ASSERT_NE(rep_socket, nullptr)
    << "Failed to create REP socket: " << zmq_strerror(zmq_errno());

  int rc = zmq_bind(rep_socket, "tcp://127.0.0.1:5555");
  ASSERT_EQ(rc, 0) << "Failed to bind REP socket: "
                   << zmq_strerror(zmq_errno());

  // Create and connect REQ socket
  void* req_socket = zmq_socket(ctx, ZMQ_REQ);
  ASSERT_NE(req_socket, nullptr)
    << "Failed to create REQ socket: " << zmq_strerror(zmq_errno());

  rc = zmq_connect(req_socket, "tcp://127.0.0.1:5555");
  ASSERT_EQ(rc, 0) << "Failed to connect REQ socket: "
                   << zmq_strerror(zmq_errno());

  // Send a message
  const char* message = "Hello";
  rc = zmq_send(req_socket, message, strlen(message), 0);
  ASSERT_NE(rc, -1) << "Failed to send message: " << zmq_strerror(zmq_errno());

  // Receive the message
  char buffer[256];
  rc = zmq_recv(rep_socket, buffer, sizeof(buffer), 0);
  ASSERT_NE(rc, -1) << "Failed to receive message: "
                    << zmq_strerror(zmq_errno());
  buffer[rc] = '\0';

  EXPECT_STREQ(buffer, message);

  // Clean up
  zmq_close(rep_socket);
  zmq_close(req_socket);
}

// Test Curve security
TEST_F(ZMQTest, CurveSecurity)
{
  // Generate server keypair
  auto [server_public, server_secret] = generate_curve_keypair();

  // Create and configure server socket
  void* server = zmq_socket(ctx, ZMQ_REP);
  ASSERT_NE(server, nullptr)
    << "Failed to create server socket: " << zmq_strerror(zmq_errno());

  // Set up server security
  int as_server = 1;
  zmq_setsockopt(server, ZMQ_CURVE_SERVER, &as_server, sizeof(as_server));
  zmq_setsockopt(
    server, ZMQ_CURVE_SECRETKEY, server_secret.c_str(), server_secret.length());

  // Bind server
  int rc = zmq_bind(server, "tcp://127.0.0.1:5556");
  ASSERT_EQ(rc, 0) << "Failed to bind server: " << zmq_strerror(zmq_errno());

  // Generate client keypair
  auto [client_public, client_secret] = generate_curve_keypair();

  // Create and configure client socket
  void* client = zmq_socket(ctx, ZMQ_REQ);
  ASSERT_NE(client, nullptr)
    << "Failed to create client socket: " << zmq_strerror(zmq_errno());

  // Set up client security
  as_server = 0;
  zmq_setsockopt(client, ZMQ_CURVE_SERVER, &as_server, sizeof(as_server));
  zmq_setsockopt(
    client, ZMQ_CURVE_PUBLICKEY, client_public.c_str(), client_public.length());
  zmq_setsockopt(
    client, ZMQ_CURVE_SECRETKEY, client_secret.c_str(), client_secret.length());
  zmq_setsockopt(
    client, ZMQ_CURVE_SERVERKEY, server_public.c_str(), server_public.length());

  // Connect client
  rc = zmq_connect(client, "tcp://127.0.0.1:5556");
  ASSERT_EQ(rc, 0) << "Failed to connect client: " << zmq_strerror(zmq_errno());

  // Test communication
  const char* test_message = "Secure message";
  rc = zmq_send(client, test_message, strlen(test_message), 0);
  ASSERT_NE(rc, -1) << "Failed to send secure message: "
                    << zmq_strerror(zmq_errno());

  char buffer[256];
  rc = zmq_recv(server, buffer, sizeof(buffer), 0);
  ASSERT_NE(rc, -1) << "Failed to receive secure message: "
                    << zmq_strerror(zmq_errno());
  buffer[rc] = '\0';

  EXPECT_STREQ(buffer, test_message);

  // Clean up
  zmq_close(server);
  zmq_close(client);
}

// Test PUB/SUB pattern with curve security
TEST_F(ZMQTest, PubSubWithCurve)
{
  // Generate server keypair
  auto [server_public, server_secret] = generate_curve_keypair();

  // Create and configure publisher
  void* publisher = zmq_socket(ctx, ZMQ_PUB);
  ASSERT_NE(publisher, nullptr)
    << "Failed to create publisher: " << zmq_strerror(zmq_errno());

  // Set up publisher security
  int as_server = 1;
  zmq_setsockopt(publisher, ZMQ_CURVE_SERVER, &as_server, sizeof(as_server));
  zmq_setsockopt(publisher,
                 ZMQ_CURVE_SECRETKEY,
                 server_secret.c_str(),
                 server_secret.length());

  // Bind publisher
  int rc = zmq_bind(publisher, "tcp://127.0.0.1:5557");
  ASSERT_EQ(rc, 0) << "Failed to bind publisher: " << zmq_strerror(zmq_errno());

  // Generate client keypair
  auto [client_public, client_secret] = generate_curve_keypair();

  // Create and configure subscriber
  void* subscriber = zmq_socket(ctx, ZMQ_SUB);
  ASSERT_NE(subscriber, nullptr)
    << "Failed to create subscriber: " << zmq_strerror(zmq_errno());

  // Set up subscriber security
  as_server = 0;
  zmq_setsockopt(subscriber, ZMQ_CURVE_SERVER, &as_server, sizeof(as_server));
  zmq_setsockopt(subscriber,
                 ZMQ_CURVE_PUBLICKEY,
                 client_public.c_str(),
                 client_public.length());
  zmq_setsockopt(subscriber,
                 ZMQ_CURVE_SECRETKEY,
                 client_secret.c_str(),
                 client_secret.length());
  zmq_setsockopt(subscriber,
                 ZMQ_CURVE_SERVERKEY,
                 server_public.c_str(),
                 server_public.length());

  // Subscribe to all messages
  rc = zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "", 0);
  ASSERT_EQ(rc, 0) << "Failed to set subscription: "
                   << zmq_strerror(zmq_errno());

  // Connect subscriber
  rc = zmq_connect(subscriber, "tcp://127.0.0.1:5557");
  ASSERT_EQ(rc, 0) << "Failed to connect subscriber: "
                   << zmq_strerror(zmq_errno());

  // Give the connection time to establish
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Send a message
  const char* topic = "test";
  const char* message = "Hello, subscribers!";

  // Send topic
  rc = zmq_send(publisher, topic, strlen(topic), ZMQ_SNDMORE);
  ASSERT_NE(rc, -1) << "Failed to send topic: " << zmq_strerror(zmq_errno());

  // Send message
  rc = zmq_send(publisher, message, strlen(message), 0);
  ASSERT_NE(rc, -1) << "Failed to publish message: "
                    << zmq_strerror(zmq_errno());

  // Receive topic
  char topic_buffer[256];
  rc = zmq_recv(subscriber, topic_buffer, sizeof(topic_buffer), 0);
  ASSERT_NE(rc, -1) << "Failed to receive topic: " << zmq_strerror(zmq_errno());
  topic_buffer[rc] = '\0';

  // Receive message
  char message_buffer[256];
  rc = zmq_recv(subscriber, message_buffer, sizeof(message_buffer), 0);
  ASSERT_NE(rc, -1) << "Failed to receive message: "
                    << zmq_strerror(zmq_errno());
  message_buffer[rc] = '\0';

  EXPECT_STREQ(topic_buffer, topic);
  EXPECT_STREQ(message_buffer, message);

  // Clean up
  zmq_close(publisher);
  zmq_close(subscriber);
}

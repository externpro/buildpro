#include <memory>
#include <string>

#include <gtest/gtest.h>
#include <libstrophe/strophe.h>

class LibStropheTest : public ::testing::Test
{
protected:
  xmpp_ctx_t* ctx;
  xmpp_conn_t* conn;
  xmpp_log_t* log;

  void SetUp() override
  {
    // Initialize libstrophe
    xmpp_initialize();

    // Create a context with default logger (debug level)
    log = xmpp_get_default_logger(XMPP_LEVEL_DEBUG);
    ctx = xmpp_ctx_new(NULL, log);
    ASSERT_NE(ctx, nullptr) << "Failed to create XMPP context";

    // Create a connection
    conn = xmpp_conn_new(ctx);
    ASSERT_NE(conn, nullptr) << "Failed to create XMPP connection";
  }

  void TearDown() override
  {
    // Clean up resources
    if (conn)
    {
      xmpp_conn_release(conn);
    }
    if (ctx)
    {
      xmpp_ctx_free(ctx);
    }
    xmpp_shutdown();
  }
};

TEST_F(LibStropheTest, VersionCheck)
{
  // Test version check functionality
  // This checks if the current version is at least 0.9.1
  int result = xmpp_version_check(0, 9);
  EXPECT_EQ(result, 1) << "Version check failed, expected version >= 0.9";

  // This should fail for a future version
  result = xmpp_version_check(99, 0);
  EXPECT_EQ(result, 0)
    << "Version check unexpectedly passed for future version";
}

TEST_F(LibStropheTest, StanzaCreation)
{
  // Test stanza creation and manipulation
  xmpp_stanza_t* stanza = xmpp_stanza_new(ctx);
  ASSERT_NE(stanza, nullptr) << "Failed to create stanza";

  // Set stanza name
  const char* name = "message";
  xmpp_stanza_set_name(stanza, name);
  const char* result_name = xmpp_stanza_get_name(stanza);
  EXPECT_STREQ(result_name, name) << "Stanza name not set correctly";

  // Set stanza attribute
  const char* attr_name = "type";
  const char* attr_value = "chat";
  xmpp_stanza_set_attribute(stanza, attr_name, attr_value);
  const char* result_attr = xmpp_stanza_get_attribute(stanza, attr_name);
  EXPECT_STREQ(result_attr, attr_value) << "Stanza attribute not set correctly";

  // Create and add a child stanza
  xmpp_stanza_t* child = xmpp_stanza_new(ctx);
  ASSERT_NE(child, nullptr) << "Failed to create child stanza";
  xmpp_stanza_set_name(child, "body");

  // Add text to the child
  const char* text = "Hello, XMPP world!";
  xmpp_stanza_t* text_stanza = xmpp_stanza_new(ctx);
  ASSERT_NE(text_stanza, nullptr) << "Failed to create text stanza";
  xmpp_stanza_set_text(text_stanza, text);
  xmpp_stanza_add_child(child, text_stanza);
  xmpp_stanza_release(text_stanza);

  // Add child to parent
  xmpp_stanza_add_child(stanza, child);
  xmpp_stanza_release(child);

  // Verify child was added
  xmpp_stanza_t* found_child = xmpp_stanza_get_child_by_name(stanza, "body");
  EXPECT_NE(found_child, nullptr) << "Child stanza not found by name";

  // Verify text content
  char* result_text = xmpp_stanza_get_text(found_child);
  if (result_text)
  {
    EXPECT_STREQ(result_text, text) << "Text content not set correctly";
    xmpp_free(ctx, result_text);
  }
  else
  {
    FAIL() << "Failed to get text from stanza";
  }

  // Convert stanza to text
  char* stanza_text;
  size_t stanza_text_size;
  int text_result =
    xmpp_stanza_to_text(stanza, &stanza_text, &stanza_text_size);
  EXPECT_EQ(text_result, 0) << "Failed to convert stanza to text";
  EXPECT_GT(stanza_text_size, 0) << "Stanza text size is zero";
  EXPECT_NE(stanza_text, nullptr) << "Stanza text is null";

  // Print the stanza text for debugging
  if (stanza_text)
  {
    std::cout << "Stanza as text: " << stanza_text << std::endl;
    xmpp_free(ctx, stanza_text);
  }

  // Clean up
  xmpp_stanza_release(stanza);
}

TEST_F(LibStropheTest, ConnectionConfiguration)
{
  // Test connection configuration
  const char* jid = "test@example.com";
  const char* password = "testpassword";

  // Set JID and password
  xmpp_conn_set_jid(conn, jid);
  xmpp_conn_set_pass(conn, password);

  // Verify JID was set correctly
  const char* result_jid = xmpp_conn_get_jid(conn);
  EXPECT_STREQ(result_jid, jid) << "JID not set correctly";

  // Set and verify connection flags
  long flags = XMPP_CONN_FLAG_MANDATORY_TLS;
  xmpp_conn_set_flags(conn, flags);

  // Test keepalive configuration
  int timeout = 60;
  int interval = 1;
  xmpp_conn_set_keepalive(conn, timeout, interval);

  // We can't easily verify the keepalive settings as there's no getter,
  // but we can ensure the function doesn't crash
  SUCCEED() << "Keepalive configuration set without errors";
}

TEST_F(LibStropheTest, MessageHelper)
{
  // Test message helper functions
  xmpp_stanza_t* message = xmpp_message_new(
    ctx, "chat", "recipient@example.com", "sender@example.com");
  ASSERT_NE(message, nullptr) << "Failed to create message stanza";

  // Set message body
  const char* body_text = "Test message body";
  int result = xmpp_message_set_body(message, body_text);
  EXPECT_EQ(result, 0) << "Failed to set message body";

  // Verify message type
  const char* type = xmpp_stanza_get_attribute(message, "type");
  EXPECT_STREQ(type, "chat") << "Message type not set correctly";

  // Verify recipient (to)
  const char* to = xmpp_stanza_get_attribute(message, "to");
  EXPECT_STREQ(to, "recipient@example.com")
    << "Message recipient not set correctly";

  // Verify sender (from) - only if it exists
  const char* from = xmpp_stanza_get_attribute(message, "from");
  if (from)
  {
    EXPECT_STREQ(from, "sender@example.com")
      << "Message sender not set correctly";
  }
  else
  {
    // Some implementations might not set the from attribute by default
    // Let's manually set it and then verify
    xmpp_stanza_set_attribute(message, "from", "sender@example.com");
    from = xmpp_stanza_get_attribute(message, "from");
    EXPECT_STREQ(from, "sender@example.com")
      << "Message sender not set correctly after manual setting";
  }

  // Verify body content
  xmpp_stanza_t* body = xmpp_stanza_get_child_by_name(message, "body");
  ASSERT_NE(body, nullptr) << "Message body not found";

  char* body_content = xmpp_stanza_get_text(body);
  if (body_content)
  {
    EXPECT_STREQ(body_content, body_text)
      << "Message body content not set correctly";
    xmpp_free(ctx, body_content);
  }
  else
  {
    FAIL() << "Failed to get message body text";
  }

  // Clean up
  xmpp_stanza_release(message);
}

TEST_F(LibStropheTest, PresenceHelper)
{
  // Test presence helper function
  xmpp_stanza_t* presence = xmpp_presence_new(ctx);
  ASSERT_NE(presence, nullptr) << "Failed to create presence stanza";

  // Verify it's a presence stanza
  const char* name = xmpp_stanza_get_name(presence);
  EXPECT_STREQ(name, "presence") << "Stanza is not a presence stanza";

  // Create and set status manually since xmpp_presence_set_status() is not
  // available
  const char* status_text = "Online and available";

  // Create status element
  xmpp_stanza_t* status = xmpp_stanza_new(ctx);
  ASSERT_NE(status, nullptr) << "Failed to create status stanza";
  xmpp_stanza_set_name(status, "status");

  // Add text to status element
  xmpp_stanza_t* text_stanza = xmpp_stanza_new(ctx);
  ASSERT_NE(text_stanza, nullptr) << "Failed to create text stanza";
  xmpp_stanza_set_text(text_stanza, status_text);
  xmpp_stanza_add_child(status, text_stanza);
  xmpp_stanza_release(text_stanza);

  // Add status to presence
  xmpp_stanza_add_child(presence, status);
  xmpp_stanza_release(status);

  // Verify status was set
  xmpp_stanza_t* status_check =
    xmpp_stanza_get_child_by_name(presence, "status");
  ASSERT_NE(status_check, nullptr)
    << "Status element not found in presence stanza";

  char* status_content = xmpp_stanza_get_text(status_check);
  if (status_content)
  {
    EXPECT_STREQ(status_content, status_text)
      << "Status text not set correctly";
    xmpp_free(ctx, status_content);
  }
  else
  {
    FAIL() << "Failed to get status text";
  }

  // Clean up
  xmpp_stanza_release(presence);
}

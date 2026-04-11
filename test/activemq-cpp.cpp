#include <iostream>
#include <memory>

#include <activemq/library/ActiveMQCPP.h>
#include <cms/ConnectionFactory.h>
#include <gtest/gtest.h>

// Simple tests for ActiveMQCPP library

TEST(ActiveMQCPPTest, LibraryInitialization)
{
  // Test that we can initialize and shutdown the ActiveMQ library
  EXPECT_NO_THROW({
    activemq::library::ActiveMQCPP::initializeLibrary();
    activemq::library::ActiveMQCPP::shutdownLibrary();
  });
}

TEST(ActiveMQCPPTest, ConnectionFactoryCreation)
{
  // Initialize the ActiveMQ library
  activemq::library::ActiveMQCPP::initializeLibrary();

  // Test that we can create a connection factory
  // Use failover protocol with a timeout to avoid hanging if server doesn't
  // exist
  std::unique_ptr<cms::ConnectionFactory> connectionFactory;
  EXPECT_NO_THROW({
    connectionFactory.reset(cms::ConnectionFactory::createCMSConnectionFactory(
      "failover:(tcp://localhost:61616)?timeout=100"));
  });

  // Just test that the factory was created, don't try to connect
  EXPECT_NE(connectionFactory.get(), nullptr)
    << "Failed to create connection factory";

  // Cleanup
  connectionFactory.reset();
  activemq::library::ActiveMQCPP::shutdownLibrary();
}

// Simple test to verify that the test framework is working
TEST(ActiveMQCPPTest, SanityCheck)
{
  EXPECT_TRUE(true) << "Sanity check passed";
}

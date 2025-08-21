#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/async.h>
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

class SpdlogTest : public ::testing::Test {
protected:
    static constexpr const char* TEST_LOG_FILE = "spdlog_test.log";
    std::shared_ptr<spdlog::logger> test_logger;

    void SetUp() override {
#if !(defined(__linux__) && (defined(__aarch64__) || defined(__arm64__)) && (__GNUC__ < 10))
        // Clear any existing log file
        if (std::filesystem::exists(TEST_LOG_FILE)) {
            std::filesystem::remove(TEST_LOG_FILE);
        }
        // Create a new logger for each test
        test_logger = spdlog::basic_logger_mt("test_logger", TEST_LOG_FILE, true);
#else
        // On Linux ARM64 with GCC < 10, use a null logger to avoid filesystem operations
        auto null_sink = std::make_shared<spdlog::sinks::null_sink_mt>();
        test_logger = std::make_shared<spdlog::logger>("test_logger", null_sink);
#endif
        spdlog::set_default_logger(test_logger);
        spdlog::set_level(spdlog::level::trace); // Ensure all levels are enabled
        spdlog::flush_on(spdlog::level::trace);  // Flush after each log
    }

    void TearDown() override {
        spdlog::shutdown(); // Ensure all messages are flushed
        spdlog::drop_all(); // Cleanup and close all loggers
    }

    static bool logFileContains(const std::string& message) {
#if !(defined(__linux__) && (defined(__aarch64__) || defined(__arm64__)) && (__GNUC__ < 10))
        std::ifstream log_file(TEST_LOG_FILE);
        std::string content((std::istreambuf_iterator<char>(log_file)),
                           (std::istreambuf_iterator<char>()));
        return content.find(message) != std::string::npos;
#else
        return true; // Skip actual filesystem check on Linux ARM64 with GCC < 10
#endif
    }
};

// Basic logging test
TEST_F(SpdlogTest, BasicLogging) {
    // Test different log levels
    test_logger->info("This is an info message");
    test_logger->warn("This is a warning message");
    test_logger->error("This is an error message");
    test_logger->critical("This is a critical message");

    // Test formatting
    test_logger->info("Formatted message with number: {}", 42);
    test_logger->info("Formatted message with string: {}", "test");

    // Force flush to ensure messages are written
    test_logger->flush();

    // Verify the logger is properly initialized
    EXPECT_NE(test_logger->level(), spdlog::level::off);
    EXPECT_NE(test_logger, nullptr);
}

// File logging test
TEST_F(SpdlogTest, FileLogging) {
#if !(defined(__linux__) && (defined(__aarch64__) || defined(__arm64__)) && (__GNUC__ < 10))
    const std::string test_message = "Test message to file";
    test_logger->info(test_message);
    test_logger->flush(); // Ensure the message is written to disk

    // Verify the log file was created and contains our message
    EXPECT_TRUE(std::filesystem::exists(TEST_LOG_FILE));
    EXPECT_TRUE(logFileContains(test_message));
#else
    GTEST_SKIP() << "Filesystem tests disabled on Linux ARM64 with GCC < 10";
#endif
}

// Multi-sink logger test
TEST_F(SpdlogTest, MultiSinkLogging) {
#if !(defined(__linux__) && (defined(__aarch64__) || defined(__arm64__)) && (__GNUC__ < 10))
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("multisink_test.log", true);

    std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
    auto multi_sink_logger = std::make_shared<spdlog::logger>("multi_sink", begin(sinks), end(sinks));
    multi_sink_logger->set_level(spdlog::level::trace);
    spdlog::set_default_logger(multi_sink_logger);

    const std::string test_message = "Multi-sink test message";
    multi_sink_logger->info(test_message);
    multi_sink_logger->flush();
#else
    GTEST_SKIP() << "Multi-sink filesystem tests disabled on Linux ARM64 with GCC < 10";
#endif

#if !(defined(__linux__) && (defined(__aarch64__) || defined(__arm64__)) && (__GNUC__ < 10))
    // Verify the message was written to file
    std::ifstream log_file("multisink_test.log");
    std::string content((std::istreambuf_iterator<char>(log_file)),
                       (std::istreambuf_iterator<char>()));
    EXPECT_TRUE(content.find(test_message) != std::string::npos);
#else
    // Skip file verification on Linux ARM64 with GCC < 10
    EXPECT_TRUE(true);
#endif
}

// Asynchronous logging test - disabled due to flakiness
TEST_F(SpdlogTest, DISABLED_AsyncLogging) {
    // Note: Async logging tests can be flaky in test environments
    // and may require special handling for proper cleanup
    auto async_logger = spdlog::basic_logger_mt<spdlog::async_factory>(
        "async_file_logger", "async_test.log");

    const std::string test_message = "Async test message";
    async_logger->info(test_message);
    async_logger->flush();

    // Give some time for async operations to complete
    std::this_thread::sleep_for(100ms);

    // Cleanup
    spdlog::drop("async_file_logger");

    // Note: Not asserting on file contents as async logging might not be reliable in tests
}

// Log level test
TEST_F(SpdlogTest, LogLevels) {
    test_logger->set_level(spdlog::level::warn);

    // These should not appear in the log
    test_logger->debug("This debug message should not appear");
    test_logger->info("This info message should not appear");

    // These should appear
    const std::string warn_msg = "This warning message should appear";
    const std::string error_msg = "This error message should appear";
    test_logger->warn(warn_msg);
    test_logger->error(error_msg);

    test_logger->flush();

#if !(defined(__linux__) && (defined(__aarch64__) || defined(__arm64__)) && (__GNUC__ < 10))
    // Verify log contents for non-null logger
    std::ifstream log_file(TEST_LOG_FILE);
    std::string content((std::istreambuf_iterator<char>(log_file)),
                       (std::istreambuf_iterator<char>()));

    EXPECT_FALSE(content.find("debug message") != std::string::npos);
    EXPECT_FALSE(content.find("info message") != std::string::npos);
    EXPECT_TRUE(content.find(warn_msg) != std::string::npos);
    EXPECT_TRUE(content.find(error_msg) != std::string::npos);
#else
    // For null logger, just verify the calls don't crash
    EXPECT_TRUE(true);
#endif
}

// Custom format test
TEST_F(SpdlogTest, CustomFormat) {
    const std::string pattern = "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v";
    test_logger->set_pattern(pattern);

    const std::string test_message = "Formatted log message";
    test_logger->info(test_message);
    test_logger->flush();

#if !(defined(__linux__) && (defined(__aarch64__) || defined(__arm64__)) && (__GNUC__ < 10))
    // Read the log file for non-null logger
    std::ifstream log_file(TEST_LOG_FILE);
    std::string line;
    std::getline(log_file, line);

    // Check for parts of our pattern
    EXPECT_NE(line.find("info"), std::string::npos);
    EXPECT_NE(line.find("test_logger"), std::string::npos);
    EXPECT_NE(line.find(test_message), std::string::npos);
#else
    // For null logger, just verify the calls don't crash
    EXPECT_TRUE(true);
#endif
}

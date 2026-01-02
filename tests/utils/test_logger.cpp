#include <gtest/gtest.h>

#include "utils/Logger.hpp"

namespace pas::utils::tests {

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Ensure logger is shutdown before each test
        if (Logger::isInitialized()) {
            Logger::shutdown();
        }
    }

    void TearDown() override {
        if (Logger::isInitialized()) {
            Logger::shutdown();
        }
    }
};

TEST_F(LoggerTest, InitializesSuccessfully) {
    EXPECT_FALSE(Logger::isInitialized());

    Logger::init("TestApp", Logger::Level::Info);

    EXPECT_TRUE(Logger::isInitialized());
}

TEST_F(LoggerTest, ShutdownCleansUp) {
    Logger::init("TestApp", Logger::Level::Info);
    EXPECT_TRUE(Logger::isInitialized());

    Logger::shutdown();
    EXPECT_FALSE(Logger::isInitialized());
}

TEST_F(LoggerTest, DoubleInitDoesNotCrash) {
    Logger::init("TestApp1");
    Logger::init("TestApp2"); // Should be ignored

    EXPECT_TRUE(Logger::isInitialized());
}

TEST_F(LoggerTest, DoubleShutdownDoesNotCrash) {
    Logger::init("TestApp");

    Logger::shutdown();
    Logger::shutdown(); // Should be safe

    EXPECT_FALSE(Logger::isInitialized());
}

TEST_F(LoggerTest, SetLevelWorks) {
    Logger::init("TestApp", Logger::Level::Info);

    Logger::setLevel(Logger::Level::Debug);
    EXPECT_EQ(Logger::getLevel(), Logger::Level::Debug);

    Logger::setLevel(Logger::Level::Error);
    EXPECT_EQ(Logger::getLevel(), Logger::Level::Error);
}

TEST_F(LoggerTest, LoggingWithoutInitDoesNotCrash) {
    // Logging before init should not crash
    Logger::info("This message should be safely ignored");
    Logger::error("This error should also be ignored");

    EXPECT_FALSE(Logger::isInitialized());
}

TEST_F(LoggerTest, AllLogLevelsWork) {
    Logger::init("TestApp", Logger::Level::Trace);

    // All these should not crash
    Logger::trace("Trace message");
    Logger::debug("Debug message");
    Logger::info("Info message");
    Logger::warn("Warning message");
    Logger::error("Error message");
    Logger::critical("Critical message");

    EXPECT_TRUE(Logger::isInitialized());
}

TEST_F(LoggerTest, FormatStringsWork) {
    Logger::init("TestApp", Logger::Level::Debug);

    // Test format strings with various types
    Logger::info("Integer: {}", 42);
    Logger::info("Float: {:.2f}", 3.14159);
    Logger::info("String: {}", "hello");
    Logger::info("Multiple: {} {} {}", 1, 2.0, "three");

    EXPECT_TRUE(Logger::isInitialized());
}

TEST_F(LoggerTest, MacrosWork) {
    Logger::init("TestApp", Logger::Level::Trace);

    // Test the convenience macros
    PAS_TRACE("Trace via macro");
    PAS_DEBUG("Debug via macro");
    PAS_INFO("Info via macro: {}", 123);
    PAS_WARN("Warning via macro");
    PAS_ERROR("Error via macro");
    PAS_CRITICAL("Critical via macro");

    EXPECT_TRUE(Logger::isInitialized());
}

TEST_F(LoggerTest, GetLevelReturnsOffWhenNotInitialized) {
    EXPECT_FALSE(Logger::isInitialized());
    EXPECT_EQ(Logger::getLevel(), Logger::Level::Off);
}

} // namespace pas::utils::tests

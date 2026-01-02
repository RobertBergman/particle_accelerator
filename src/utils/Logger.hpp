#pragma once

#include <memory>
#include <string>
#include <string_view>

#include <spdlog/spdlog.h>

namespace pas::utils {

/**
 * @brief Logger wrapper using the Adapter Pattern.
 *
 * Provides a simplified interface to spdlog for consistent logging
 * throughout the application.
 */
class Logger {
public:
    enum class Level {
        Trace,
        Debug,
        Info,
        Warn,
        Error,
        Critical,
        Off
    };

    /**
     * @brief Initialize the logging system.
     * @param appName Name of the application (used in log prefix).
     * @param level Minimum log level to output.
     */
    static void init(std::string_view appName = "PAS", Level level = Level::Info);

    /**
     * @brief Shutdown the logging system.
     */
    static void shutdown();

    /**
     * @brief Set the minimum log level.
     */
    static void setLevel(Level level);

    /**
     * @brief Get the current log level.
     */
    static Level getLevel();

    /**
     * @brief Check if the logger is initialized.
     */
    static bool isInitialized();

    // Log methods
    template<typename... Args>
    static void trace(spdlog::format_string_t<Args...> fmt, Args&&... args) {
        if (s_logger) {
            s_logger->trace(fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    static void debug(spdlog::format_string_t<Args...> fmt, Args&&... args) {
        if (s_logger) {
            s_logger->debug(fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    static void info(spdlog::format_string_t<Args...> fmt, Args&&... args) {
        if (s_logger) {
            s_logger->info(fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    static void warn(spdlog::format_string_t<Args...> fmt, Args&&... args) {
        if (s_logger) {
            s_logger->warn(fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    static void error(spdlog::format_string_t<Args...> fmt, Args&&... args) {
        if (s_logger) {
            s_logger->error(fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    static void critical(spdlog::format_string_t<Args...> fmt, Args&&... args) {
        if (s_logger) {
            s_logger->critical(fmt, std::forward<Args>(args)...);
        }
    }

private:
    static std::shared_ptr<spdlog::logger> s_logger;
    static bool s_initialized;
};

} // namespace pas::utils

// Convenience macros
#define PAS_TRACE(...)    ::pas::utils::Logger::trace(__VA_ARGS__)
#define PAS_DEBUG(...)    ::pas::utils::Logger::debug(__VA_ARGS__)
#define PAS_INFO(...)     ::pas::utils::Logger::info(__VA_ARGS__)
#define PAS_WARN(...)     ::pas::utils::Logger::warn(__VA_ARGS__)
#define PAS_ERROR(...)    ::pas::utils::Logger::error(__VA_ARGS__)
#define PAS_CRITICAL(...) ::pas::utils::Logger::critical(__VA_ARGS__)

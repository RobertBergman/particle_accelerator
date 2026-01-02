#include "utils/Logger.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace pas::utils {

std::shared_ptr<spdlog::logger> Logger::s_logger = nullptr;
bool Logger::s_initialized = false;

namespace {

spdlog::level::level_enum toSpdlogLevel(Logger::Level level) {
    switch (level) {
        case Logger::Level::Trace:    return spdlog::level::trace;
        case Logger::Level::Debug:    return spdlog::level::debug;
        case Logger::Level::Info:     return spdlog::level::info;
        case Logger::Level::Warn:     return spdlog::level::warn;
        case Logger::Level::Error:    return spdlog::level::err;
        case Logger::Level::Critical: return spdlog::level::critical;
        case Logger::Level::Off:      return spdlog::level::off;
        default:                      return spdlog::level::info;
    }
}

Logger::Level fromSpdlogLevel(spdlog::level::level_enum level) {
    switch (level) {
        case spdlog::level::trace:    return Logger::Level::Trace;
        case spdlog::level::debug:    return Logger::Level::Debug;
        case spdlog::level::info:     return Logger::Level::Info;
        case spdlog::level::warn:     return Logger::Level::Warn;
        case spdlog::level::err:      return Logger::Level::Error;
        case spdlog::level::critical: return Logger::Level::Critical;
        case spdlog::level::off:      return Logger::Level::Off;
        default:                      return Logger::Level::Info;
    }
}

} // anonymous namespace

void Logger::init(std::string_view appName, Level level) {
    if (s_initialized) {
        return;
    }

    // Create console sink with colors
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");

    // Create the logger with the console sink
    s_logger = std::make_shared<spdlog::logger>(std::string(appName), consoleSink);
    s_logger->set_level(toSpdlogLevel(level));
    s_logger->flush_on(spdlog::level::warn);

    // Register as default logger
    spdlog::set_default_logger(s_logger);

    s_initialized = true;
}

void Logger::shutdown() {
    if (!s_initialized) {
        return;
    }

    s_logger->flush();
    spdlog::shutdown();
    s_logger.reset();
    s_initialized = false;
}

void Logger::setLevel(Level level) {
    if (s_logger) {
        s_logger->set_level(toSpdlogLevel(level));
    }
}

Logger::Level Logger::getLevel() {
    if (s_logger) {
        return fromSpdlogLevel(s_logger->level());
    }
    return Level::Off;
}

bool Logger::isInitialized() {
    return s_initialized;
}

} // namespace pas::utils

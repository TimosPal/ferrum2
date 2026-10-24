#pragma once

#include <cstdint>
#include <cstdio>
#include <format>
#include <print>
#include <string_view>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include "ferrum/common/timestamp.hpp"

namespace ferrum::common {

// Severity of a log record, from least to most severe.
enum class LogLevel : std::uint8_t { Trace, Info, Warn, Error, Critical };

namespace detail {

inline LogLevel& CurrentLevel() noexcept {
    static LogLevel level = LogLevel::Info;
    return level;
}

inline bool ShouldLog(LogLevel level) noexcept { return level >= CurrentLevel(); }

inline std::string_view LogLevelName(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::Trace:
            return "TRACE";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warn:
            return "WARN";
        case LogLevel::Error:
            return "ERROR";
        case LogLevel::Critical:
            return "CRITICAL";
    }
    return "UNKNOWN";
}

inline std::string_view LogLevelColor(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::Trace:
            return "\x1b[90m";  // grey
        case LogLevel::Info:
            return "\x1b[32m";  // green
        case LogLevel::Warn:
            return "\x1b[33m";  // yellow
        case LogLevel::Error:
            return "\x1b[31m";  // red
        case LogLevel::Critical:
            return "\x1b[1;31m";  // bold red
    }
    return "\x1b[0m";
}

// Colour only when the stream is attached to a terminal, so redirected output
// stays clean.
inline bool SupportsColor(FILE* stream) noexcept {
#ifdef _WIN32
    return ::_isatty(::_fileno(stream)) != 0;
#else
    return ::isatty(::fileno(stream)) != 0;
#endif
}

// Writes a single log record. Error and above go to stderr.
inline void Log(LogLevel level, std::string_view message) {
    const bool to_stderr = level == LogLevel::Error || level == LogLevel::Critical;
    FILE* stream = to_stderr ? stderr : stdout;
    const std::string_view color = SupportsColor(stream) ? LogLevelColor(level) : "";
    const std::string_view reset = color.empty() ? "" : "\x1b[0m";
    std::print(stream, "{}[{}] [{}] {}{}\n", color, CurrentTimestamp(), LogLevelName(level),
               message, reset);
}

}  // namespace detail

// Sets the minimum level that is written. Call once at startup
inline void SetLogLevel(LogLevel level) noexcept { detail::CurrentLevel() = level; }

}  // namespace ferrum::common

#define FE_LOG(level, ...)                                                  \
    do {                                                                    \
        if (::ferrum::common::detail::ShouldLog(level)) {                   \
            ::ferrum::common::detail::Log(level, std::format(__VA_ARGS__)); \
        }                                                                   \
    } while (false)

#ifdef NDEBUG
#define FE_TRACE(...) static_cast<void>(0)
#else
#define FE_TRACE(...) FE_LOG(::ferrum::common::LogLevel::Trace, __VA_ARGS__)
#endif
#define FE_INFO(...) FE_LOG(::ferrum::common::LogLevel::Info, __VA_ARGS__)
#define FE_WARN(...) FE_LOG(::ferrum::common::LogLevel::Warn, __VA_ARGS__)
#define FE_ERROR(...) FE_LOG(::ferrum::common::LogLevel::Error, __VA_ARGS__)
#define FE_CRITICAL(...) FE_LOG(::ferrum::common::LogLevel::Critical, __VA_ARGS__)

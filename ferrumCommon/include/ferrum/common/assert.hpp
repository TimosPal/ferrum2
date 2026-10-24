#pragma once

// Platform-specific debug break.
#ifdef _MSC_VER
#include <intrin.h>
#define FE_DEBUGBREAK() __debugbreak()
#elif defined(__linux__) || defined(__APPLE__)
#include <csignal>
#define FE_DEBUGBREAK() static_cast<void>(raise(SIGABRT))
#else
#include <cstdlib>
#define FE_DEBUGBREAK() std::abort()
#endif

// Compile-time assertion.
#define FE_STATIC_ASSERT(expr, msg) static_assert((expr), msg)

#ifdef NDEBUG
#define FE_ASSERT(expr, ...) (static_cast<void>(0))
#else
#include <cstdio>
#include <format>
#include <print>
#include <source_location>
#include <string_view>

namespace ferrum::common::detail {

// Reports a failed assertion and terminates.
[[noreturn]] inline void AssertFailed(
    std::string_view expression, std::string_view message, std::string_view function,
    const std::source_location& location = std::source_location::current()) {
    std::print(stderr,
               "=====================================\n"
               "ASSERTION FAILED\n"
               "Expression: {}\n"
               "Message: {}\n"
               "Function: {}\n"
               "File: {}:{}\n"
               "=====================================\n",
               expression, message, function, location.file_name(), location.line());
    FE_DEBUGBREAK();
    std::abort();
}

}  // namespace ferrum::common::detail

// FE_ASSERT(cond, "message") or FE_ASSERT(cond, "x = {}", x).
#define FE_ASSERT(expr, ...)       \
    ((expr) ? static_cast<void>(0) \
            : ::ferrum::common::detail::AssertFailed(#expr, std::format(__VA_ARGS__), __func__))
#endif

#pragma once

#include <format>
#include <source_location>
#include <stdexcept>
#include <string>

namespace ferrum::common {

// Base class for all Ferrum exceptions. Records where it was constructed.
class Exception : public std::runtime_error {
public:
    explicit Exception(const std::string& message,
                       const std::source_location& location = std::source_location::current())
        : std::runtime_error(message), location_(location) {}

    // Source location the exception was created at.
    const std::source_location& Where() const noexcept { return location_; }

private:
    std::source_location location_;
};

}  // namespace ferrum::common

#define FE_THROW(...) throw ::ferrum::common::Exception(std::format(__VA_ARGS__))

#define FE_THROW_IF(condition, ...) ((condition) ? FE_THROW(__VA_ARGS__) : static_cast<void>(0))

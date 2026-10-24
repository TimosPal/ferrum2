#pragma once

#include <chrono>
#include <format>
#include <string>

namespace ferrum::common {

inline std::string CurrentTimestamp() {
    const auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    return std::format("{:%H:%M:%S}", now);
}

}  // namespace ferrum::common

#pragma once

#include <chrono>
#include <string>
#include <string_view>

#include "ferrum/common/log.hpp"
#include "ferrum/common/timestamp.hpp"

namespace ferrum::common {

// Monotonic clock for measuring elapsed time.
using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;

// A simple stopwatch for measuring elapsed time.
class Timer {
public:
    Timer() noexcept { Reset(); }

    // Resets the timer to start now.
    void Reset() noexcept { start_ = Clock::now(); }

    // Returns elapsed time in seconds as double.
    double ElapsedSeconds() const noexcept {
        return std::chrono::duration<double>(Clock::now() - start_).count();
    }

    // Returns elapsed time as nanoseconds.
    std::chrono::nanoseconds ElapsedNanoseconds() const noexcept {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - start_);
    }

    // Returns the raw elapsed duration.
    Clock::duration Elapsed() const noexcept { return Clock::now() - start_; }

private:
    TimePoint start_;
};

class ScopedTimer {
public:
    explicit ScopedTimer(std::string_view name) noexcept : name_(name) {}

    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;
    ScopedTimer(ScopedTimer&&) = delete;
    ScopedTimer& operator=(ScopedTimer&&) = delete;

    ~ScopedTimer() noexcept {
        const auto elapsed = timer_.ElapsedNanoseconds();
        FE_TRACE("[{}] Elapsed time: {} ns", name_, elapsed.count());
    }

private:
    std::string name_;
    Timer timer_;
};

}  // namespace ferrum::common

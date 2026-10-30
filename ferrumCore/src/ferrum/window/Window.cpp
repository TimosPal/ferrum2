#include "ferrum/window/Window.hpp"

#include <exception>
#include <utility>

namespace ferrum::window {

void Window::NotifyRefresh() noexcept {
    if (!refresh_callback_) {
        return;
    }

    try {
        refresh_callback_();
    } catch (...) {
        if (!refresh_exception_) {
            refresh_exception_ = std::current_exception();
        }
    }
}

void Window::RethrowRefreshException() {
    if (refresh_exception_) {
        auto exception = std::exchange(refresh_exception_, nullptr);
        std::rethrow_exception(exception);
    }
}

}  // namespace ferrum::window

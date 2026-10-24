#include "ferrum/window/Window.hpp"

#include <exception>
#include <utility>

namespace ferrum::window {

void Window::NotifyRefresh() noexcept {
    if (!refreshCallback_) {
        return;
    }

    try {
        refreshCallback_();
    } catch (...) {
        if (!refreshException_) {
            refreshException_ = std::current_exception();
        }
    }
}

void Window::RethrowRefreshException() {
    if (refreshException_) {
        auto exception = std::exchange(refreshException_, nullptr);
        std::rethrow_exception(exception);
    }
}

}  // namespace ferrum::window

#pragma once

#include <exception>
#include <functional>
#include <memory>
#include <utility>

#include "WindowDesc.hpp"
#include "ferrum/graphics/WindowGraphicsBinding.hpp"

namespace ferrum::window {

class Window {
public:
    using RefreshCallback = std::function<void()>;

    Window(WindowDesc desc) : desc_(std::move(desc)) {}
    virtual ~Window() = default;
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    const WindowDesc& GetDesc() const noexcept { return desc_; }
    virtual void SetDesc(WindowDesc desc) { desc_ = std::move(desc); }

    ferrum::graphics::WindowGraphicsBinding* GetGraphicsBinding() noexcept {
        return graphicsBinding_.get();
    }
    void SetRefreshCallback(RefreshCallback callback) { refreshCallback_ = std::move(callback); }
    virtual void PollEvents() = 0;
    virtual bool ShouldClose() const = 0;

protected:
    // Notify the window that it needs to refresh its contents.
    // This will call the refresh callback if it is set and capture any exceptions.
    // If an exception occurs, it will be stored and can be rethrown later using
    // RethrowRefreshException().
    void NotifyRefresh() noexcept;
    void RethrowRefreshException();

    std::unique_ptr<ferrum::graphics::WindowGraphicsBinding> graphicsBinding_;
    WindowDesc desc_;

private:
    RefreshCallback refreshCallback_;
    std::exception_ptr refreshException_;
};

}  // namespace ferrum::window
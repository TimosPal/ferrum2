#pragma once

#include <memory>

#include "ferrum/graphics/GraphicsAPI.hpp"
#include "ferrum/window/Window.hpp"

struct GLFWwindow;

namespace ferrum::window::glfw {

class GLFWWindow final : public window::Window {
public:
    GLFWWindow(const WindowDesc& data, graphics::GraphicsAPI api);
    GLFWWindow(const GLFWWindow&) = delete;
    GLFWWindow& operator=(const GLFWWindow&) = delete;
    GLFWWindow(GLFWWindow&&) = delete;
    GLFWWindow& operator=(GLFWWindow&&) = delete;
    ~GLFWWindow() override;

    void PollEvents() override;
    bool ShouldClose() const override;
    glm::ivec2 GetFramebufferSize() const override;

private:
    static void RefreshCallback(GLFWwindow* window);

    struct WindowDeleter {
        void operator()(GLFWwindow* window) const noexcept;
    };

    std::unique_ptr<GLFWwindow, WindowDeleter> window_;
};

}  // namespace ferrum::window::glfw
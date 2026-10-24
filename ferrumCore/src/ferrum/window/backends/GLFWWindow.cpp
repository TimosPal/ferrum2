#include "GLFWWindow.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <memory>

#include "ferrum/common/assert.hpp"
#include "ferrum/common/exception.hpp"
#include "ferrum/graphics/backends/GLFWOpenGLWindowBinding.hpp"

namespace ferrum::window::backends {

namespace {

void ConfigureWindowHints(graphics::GraphicsAPI api) {
    glfwDefaultWindowHints();
    switch (api) {
        case graphics::GraphicsAPI::OpenGL:
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            return;
        case graphics::GraphicsAPI::Vulkan:
        case graphics::GraphicsAPI::DirectX:
            FE_THROW("Vulkan and DirectX not yet supported with GLFW");
        default:
            FE_THROW("Unknown graphics API");
    }
}

std::unique_ptr<graphics::WindowGraphicsBinding> CreateGraphicsBinding(graphics::GraphicsAPI api,
                                                                       GLFWwindow* window,
                                                                       bool vsync) {
    switch (api) {
        case graphics::GraphicsAPI::OpenGL: {
            auto binding = std::make_unique<graphics::backends::GLFWOpenGLWindowBinding>(window);
            binding->MakeCurrent();
            glfwSwapInterval(vsync ? 1 : 0);
            return binding;
        }
        case graphics::GraphicsAPI::Vulkan:
        case graphics::GraphicsAPI::DirectX:
            FE_THROW("Vulkan and DirectX not yet supported with GLFW");
        default:
            FE_THROW("Unknown graphics API");
    }
}

}  // namespace

void GLFWWindow::WindowDeleter::operator()(GLFWwindow* window) const noexcept {
    if (window != nullptr) {
        glfwDestroyWindow(window);
    }
}

GLFWWindow::GLFWWindow(const WindowDesc& data, graphics::GraphicsAPI api) : Window(data) {
    ConfigureWindowHints(api);
    window_.reset(
        glfwCreateWindow(desc_.width, desc_.height, desc_.title.c_str(), nullptr, nullptr));
    FE_THROW_IF(window_ == nullptr, "Failed to create GLFW window");

    glfwSetWindowUserPointer(window_.get(), this);
    glfwSetWindowRefreshCallback(window_.get(), RefreshCallback);
    graphicsBinding_ = CreateGraphicsBinding(api, window_.get(), desc_.vsync);
}

GLFWWindow::~GLFWWindow() { graphicsBinding_.reset(); }

void GLFWWindow::RefreshCallback(GLFWwindow* window) {
    auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
    if (self != nullptr) {
        self->NotifyRefresh();
    }
}

void GLFWWindow::PollEvents() {
    FE_ASSERT(window_ != nullptr, "GLFW window is null");
    if (window_ == nullptr) {
        return;
    }
    glfwPollEvents();
    // Since glfw is C based, it cannot propagate exceptions through its callbacks.
    // Therefore, we need to manually rethrow any exceptions captured during the refresh callback.
    RethrowRefreshException();
}

bool GLFWWindow::ShouldClose() const {
    FE_ASSERT(window_ != nullptr, "GLFW window is null");
    if (window_ == nullptr) {
        return true;
    }
    return glfwWindowShouldClose(window_.get()) != 0;
}

}  // namespace ferrum::window::backends
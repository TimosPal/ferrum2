#include "ferrum/core/backends/glfw/GLFWPlatform.hpp"

#include <GLFW/glfw3.h>

#include <memory>

#include "ferrum/common/exception.hpp"
#include "ferrum/common/log.hpp"
#include "ferrum/window/backends/glfw/GLFWWindow.hpp"

namespace ferrum::core::glfw {

namespace {

GLFWPlatform* active_platform = nullptr;

void ErrorCallback(int error, const char* description) {
    FE_ERROR("GLFW Error ({}): {}", error, description);
}

}  // namespace

GLFWPlatform::GLFWPlatform() {
    FE_THROW_IF(active_platform != nullptr, "A GLFW platform is already active");
    glfwSetErrorCallback(ErrorCallback);
    const int glfw_init_result = glfwInit();
    FE_THROW_IF(glfw_init_result != GLFW_TRUE, "Failed to initialize GLFW");
    active_platform = this;
}

GLFWPlatform::~GLFWPlatform() {
    if (active_platform == this) {
        glfwTerminate();
        active_platform = nullptr;
    }
}

std::unique_ptr<ferrum::window::Window> GLFWPlatform::CreateWindow(
    const ferrum::window::WindowDesc& data, ferrum::graphics::GraphicsAPI api) {
    return std::make_unique<ferrum::window::glfw::GLFWWindow>(data, api);
}

}  // namespace ferrum::core::glfw

namespace ferrum::core {

std::unique_ptr<Platform> Platform::Create() { return std::make_unique<glfw::GLFWPlatform>(); }

}  // namespace ferrum::core

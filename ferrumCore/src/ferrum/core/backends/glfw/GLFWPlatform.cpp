#include "ferrum/core/backends/glfw/GLFWPlatform.hpp"

#include <GLFW/glfw3.h>

#include <memory>

#include "ferrum/common/exception.hpp"
#include "ferrum/common/log.hpp"
#include "ferrum/window/backends/glfw/GLFWWindow.hpp"

namespace ferrum::core::glfw {

namespace {

GLFWPlatform* activePlatform = nullptr;

void ErrorCallback(int error, const char* description) {
    FE_ERROR("GLFW Error ({}): {}", error, description);
}

}  // namespace

GLFWPlatform::GLFWPlatform() {
    FE_THROW_IF(activePlatform != nullptr, "A GLFW platform is already active");
    glfwSetErrorCallback(ErrorCallback);
    const int glfwInitResult = glfwInit();
    FE_THROW_IF(glfwInitResult != GLFW_TRUE, "Failed to initialize GLFW");
    activePlatform = this;
}

GLFWPlatform::~GLFWPlatform() {
    if (activePlatform == this) {
        glfwTerminate();
        activePlatform = nullptr;
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

#include "ferrum/core/backends/GLFWPlatform.hpp"

#include <GLFW/glfw3.h>

#include <memory>

#include "ferrum/common/exception.hpp"
#include "ferrum/common/log.hpp"
#include "ferrum/window/backends/GLFWWindow.hpp"

namespace ferrum::core::backends {

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
    return std::make_unique<ferrum::window::backends::GLFWWindow>(data, api);
}

}  // namespace ferrum::core::backends

namespace ferrum::core {

std::unique_ptr<Platform> Platform::Create() { return std::make_unique<backends::GLFWPlatform>(); }

}  // namespace ferrum::core

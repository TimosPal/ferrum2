#include "ferrum/graphics/backends/GLFWOpenGLWindowBinding.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace ferrum::graphics::backends {

GLFWOpenGLWindowBinding::GLFWOpenGLWindowBinding(GLFWwindow* window) : window_(window) {}

void GLFWOpenGLWindowBinding::MakeCurrent() { glfwMakeContextCurrent(window_); }

OpenGLProcAddress GLFWOpenGLWindowBinding::GetProcAddress(const char* name) {
    return reinterpret_cast<OpenGLProcAddress>(glfwGetProcAddress(name));
}

glm::ivec2 GLFWOpenGLWindowBinding::GetFramebufferSize() const {
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window_, &width, &height);
    return {width, height};
}

void GLFWOpenGLWindowBinding::SwapBuffers() { glfwSwapBuffers(window_); }

}  // namespace ferrum::graphics::backends

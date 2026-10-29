#pragma once

#include "ferrum/graphics/OpenGLWindowBinding.hpp"

struct GLFWwindow;

namespace ferrum::graphics::glfw {

class GLFWOpenGLWindowBinding final : public graphics::OpenGLWindowBinding {
public:
    explicit GLFWOpenGLWindowBinding(GLFWwindow* window);

    void MakeCurrent() override;
    OpenGLProcAddress GetProcAddress(const char* name) override;
    glm::ivec2 GetFramebufferSize() const override;
    void SwapBuffers() override;

private:
    GLFWwindow* window_;
};

}  // namespace ferrum::graphics::glfw

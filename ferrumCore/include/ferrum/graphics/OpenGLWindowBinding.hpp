#pragma once

#include <glm/vec2.hpp>

#include "ferrum/graphics/WindowGraphicsBinding.hpp"

namespace ferrum::graphics {

using OpenGLProcAddress = void (*)();

class OpenGLWindowBinding : public WindowGraphicsBinding {
public:
    ~OpenGLWindowBinding() override = default;

    virtual void MakeCurrent() = 0;
    virtual OpenGLProcAddress GetProcAddress(const char* name) = 0;
    virtual glm::ivec2 GetFramebufferSize() const = 0;
    virtual void SwapBuffers() = 0;
};

}  // namespace ferrum::graphics

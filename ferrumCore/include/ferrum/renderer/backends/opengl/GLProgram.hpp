#pragma once

#include "ferrum/renderer/backends/opengl/GLShader.hpp"

namespace ferrum::renderer::opengl {

// Requires loaded OpenGL functions and a compatible current context for all GL operations.
// Destroy the program before destroying its GL context.
class GLProgram {
public:
    // Borrows compiled shaders for linking and detaches them before returning.
    // The shaders can be reused or destroyed after construction.
    GLProgram(const GLShader& vertexShader, const GLShader& fragmentShader);
    ~GLProgram();

    GLProgram(const GLProgram&) = delete;
    GLProgram& operator=(const GLProgram&) = delete;
    GLProgram(GLProgram&& other) noexcept;
    GLProgram& operator=(GLProgram&& other) noexcept;

    bool IsValid() const noexcept { return id_ != 0; }
    void Use() const;
    unsigned int Id() const noexcept { return id_; }

private:
    unsigned int id_ = 0;
};

}  // namespace ferrum::renderer::opengl

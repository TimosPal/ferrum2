#pragma once

#include <string>
#include <unordered_map>

#include "ferrum/renderer/ProgramModule.hpp"
#include "ferrum/renderer/backends/opengl/GLShader.hpp"

namespace ferrum::renderer::opengl {

// Requires loaded OpenGL functions and a compatible current context for all GL operations.
// Destroy the program before destroying its GL context.
class GLProgram final : public renderer::ProgramModule {
public:
    // Borrows compiled shaders for linking and detaches them before returning.
    // The shaders can be reused or destroyed after construction.
    GLProgram(const GLShader& vertex_shader, const GLShader& fragment_shader);
    ~GLProgram() override;

    GLProgram(const GLProgram&) = delete;
    GLProgram& operator=(const GLProgram&) = delete;
    GLProgram(GLProgram&& other) noexcept;
    GLProgram& operator=(GLProgram&& other) noexcept;

    bool IsValid() const noexcept { return id_ != 0; }
    void Use() const;
    void SetParameter(std::string_view name, const renderer::ProgramParameterValue& value) override;
    unsigned int Id() const noexcept { return id_; }

private:
    unsigned int id_ = 0;
    std::unordered_map<std::string, int> parameter_locations_;
};

}  // namespace ferrum::renderer::opengl

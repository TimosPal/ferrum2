#pragma once

#include <string_view>

#include "ferrum/renderer/ShaderModule.hpp"

namespace ferrum::renderer::opengl {

// Requires loaded OpenGL functions and a compatible current context for construction,
// destruction, and move assignment. Destroy shaders before destroying their context.
class GLShader final : public renderer::ShaderModule {
public:
    // Compiles GLSL source and throws with the compiler log on failure.
    GLShader(ShaderType type, std::string_view source);
    ~GLShader() override;

    GLShader(const GLShader&) = delete;
    GLShader& operator=(const GLShader&) = delete;
    GLShader(GLShader&& other) noexcept;
    GLShader& operator=(GLShader&& other) noexcept;

    bool IsValid() const noexcept { return id_ != 0; }
    unsigned int Id() const noexcept { return id_; }
    ShaderType GetType() const noexcept { return type_; }

private:
    GLShader() = default;
    void Compile(std::string_view source);

    ShaderType type_ = ShaderType::Vertex;
    unsigned int id_ = 0;
};

}  // namespace ferrum::renderer::opengl

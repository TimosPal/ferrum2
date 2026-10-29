#pragma once

#include "ferrum/resources/ResourceHandle.hpp"
#include "ferrum/resources/types/TextResource.hpp"

namespace ferrum::renderer::opengl {

// Requires loaded OpenGL functions and a compatible current context for construction,
// destruction, and move assignment. Destroy shaders before destroying their context.
// The resource manager must outlive the shader's retained text resource handle.
class GLShader {
public:
    enum class Type { Vertex, Fragment };

    // Compiles GLSL source and throws with the compiler log on failure.
    GLShader(Type type, resources::ResourceHandle<resources::types::TextResource> source);
    ~GLShader();

    GLShader(const GLShader&) = delete;
    GLShader& operator=(const GLShader&) = delete;
    GLShader(GLShader&& other) noexcept;
    GLShader& operator=(GLShader&& other) noexcept;

    bool IsValid() const noexcept { return id_ != 0; }
    unsigned int Id() const noexcept { return id_; }
    Type GetType() const noexcept { return type_; }

private:
    GLShader() = default;

    resources::ResourceHandle<resources::types::TextResource> source_{nullptr};
    Type type_ = Type::Vertex;
    unsigned int id_ = 0;
};

}  // namespace ferrum::renderer::opengl

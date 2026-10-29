#include "ferrum/renderer/backends/opengl/GLShader.hpp"

#include <limits>
#include <utility>

#include "ferrum/renderer/backends/opengl/utils.hpp"

namespace ferrum::renderer::opengl {
namespace {

GLenum ShaderType(GLShader::Type type) {
    switch (type) {
        case GLShader::Type::Vertex:
            return GL_VERTEX_SHADER;
        case GLShader::Type::Fragment:
            return GL_FRAGMENT_SHADER;
    }
    FE_THROW("Unsupported OpenGL shader type");
}

void CheckShaderCompilation(GLuint id, GLShader::Type type) {
    GLint compiled = GL_FALSE;
    FE_GL(glGetShaderiv(id, GL_COMPILE_STATUS, &compiled));
    if (compiled == GL_TRUE) {
        return;
    }

    FE_THROW("Failed to compile OpenGL {} shader: {}",
             type == GLShader::Type::Vertex ? "vertex" : "fragment",
             detail::GetInfoLog(id, glGetShaderiv, glGetShaderInfoLog));
}

}  // namespace

// Delegation ensures the destructor releases the shader if compilation or error checking throws.
GLShader::GLShader(Type type, resources::ResourceHandle<resources::types::TextResource> source)
    : GLShader() {
    type_ = type;
    FE_THROW_IF(!source.IsValid(), "Cannot create an OpenGL shader without a text resource");
    source_ = std::move(source);
    const auto& text = source_.GetResource()->GetText();
    FE_THROW_IF(text.size() > static_cast<std::size_t>(std::numeric_limits<GLint>::max()),
                "OpenGL shader source is too large");

    FE_GL(id_ = glCreateShader(ShaderType(type)));
    FE_THROW_IF(!IsValid(), "Failed to create OpenGL shader");

    const char* data = text.data();
    const auto length = static_cast<GLint>(text.size());
    FE_GL(glShaderSource(id_, 1, &data, &length));
    FE_GL(glCompileShader(id_));
    CheckShaderCompilation(id_, type);
}

GLShader::~GLShader() {
    if (id_ != 0) {
        glDeleteShader(id_);
    }
}

GLShader::GLShader(GLShader&& other) noexcept
    : source_(std::move(other.source_)), type_(other.type_), id_(std::exchange(other.id_, 0)) {}

GLShader& GLShader::operator=(GLShader&& other) noexcept {
    if (this != &other) {
        if (id_ != 0) {
            glDeleteShader(id_);
        }
        source_ = std::move(other.source_);
        type_ = other.type_;
        id_ = std::exchange(other.id_, 0);
    }
    return *this;
}

}  // namespace ferrum::renderer::opengl

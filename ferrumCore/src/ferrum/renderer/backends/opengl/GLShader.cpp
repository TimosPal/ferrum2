#include "ferrum/renderer/backends/opengl/GLShader.hpp"

#include <limits>
#include <utility>

#include "ferrum/renderer/backends/opengl/utils.hpp"

namespace ferrum::renderer::opengl {
namespace {

GLenum ToOpenGLShaderType(ShaderType type) {
    switch (type) {
        case ShaderType::Vertex:
            return GL_VERTEX_SHADER;
        case ShaderType::Fragment:
            return GL_FRAGMENT_SHADER;
    }
    FE_THROW("Unsupported OpenGL shader type");
}

void CheckShaderCompilation(GLuint id, ShaderType type) {
    GLint compiled = GL_FALSE;
    FE_GL(glGetShaderiv(id, GL_COMPILE_STATUS, &compiled));
    if (compiled == GL_TRUE) {
        return;
    }

    FE_THROW("Failed to compile OpenGL {} shader: {}",
             type == ShaderType::Vertex ? "vertex" : "fragment",
             detail::GetInfoLog(id, glGetShaderiv, glGetShaderInfoLog));
}

}  // namespace

GLShader::GLShader(ShaderType type, std::string_view source) : GLShader() {
    type_ = type;
    Compile(source);
}

void GLShader::Compile(std::string_view source) {
    FE_THROW_IF(source.size() > static_cast<std::size_t>(std::numeric_limits<GLint>::max()),
                "OpenGL shader source is too large");

    FE_GL(id_ = glCreateShader(ToOpenGLShaderType(type_)));
    FE_THROW_IF(!IsValid(), "Failed to create OpenGL shader");

    const char* data = source.data();
    const auto length = static_cast<GLint>(source.size());
    FE_GL(glShaderSource(id_, 1, &data, &length));
    FE_GL(glCompileShader(id_));
    CheckShaderCompilation(id_, type_);
}

GLShader::~GLShader() {
    if (id_ != 0) {
        glDeleteShader(id_);
    }
}

GLShader::GLShader(GLShader&& other) noexcept
    : type_(other.type_), id_(std::exchange(other.id_, 0)) {}

GLShader& GLShader::operator=(GLShader&& other) noexcept {
    if (this != &other) {
        if (id_ != 0) {
            glDeleteShader(id_);
        }
        type_ = other.type_;
        id_ = std::exchange(other.id_, 0);
    }
    return *this;
}

}  // namespace ferrum::renderer::opengl

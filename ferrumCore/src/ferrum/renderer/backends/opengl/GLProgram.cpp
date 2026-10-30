#include "ferrum/renderer/backends/opengl/GLProgram.hpp"

#include <utility>

#include "ferrum/renderer/backends/opengl/utils.hpp"

namespace ferrum::renderer::opengl {

namespace {

void CheckProgramLinking(GLuint id) {
    GLint linked = GL_FALSE;
    FE_GL(glGetProgramiv(id, GL_LINK_STATUS, &linked));
    if (linked == GL_TRUE) {
        return;
    }

    FE_THROW("Failed to link OpenGL program: {}",
             detail::GetInfoLog(id, glGetProgramiv, glGetProgramInfoLog));
}

}  // namespace

GLProgram::GLProgram(const GLShader& vertex_shader, const GLShader& fragment_shader) {
    detail::RequireValid(vertex_shader.IsValid(), "create OpenGL program", "vertex shader");
    detail::RequireValid(fragment_shader.IsValid(), "create OpenGL program", "fragment shader");
    FE_THROW_IF(vertex_shader.GetType() != GLShader::Type::Vertex ||
                    fragment_shader.GetType() != GLShader::Type::Fragment,
                "OpenGL program requires a vertex shader followed by a fragment shader");

    try {
        FE_GL(id_ = glCreateProgram());
        FE_THROW_IF(!IsValid(), "Failed to create OpenGL program");
        FE_GL(glAttachShader(id_, vertex_shader.Id()));
        FE_GL(glAttachShader(id_, fragment_shader.Id()));
        FE_GL(glLinkProgram(id_));
        CheckProgramLinking(id_);
        FE_GL(glDetachShader(id_, vertex_shader.Id()));
        FE_GL(glDetachShader(id_, fragment_shader.Id()));
    } catch (...) {
        if (id_ != 0) {
            glDeleteProgram(id_);
        }
        throw;
    }
}

GLProgram::~GLProgram() {
    if (id_ != 0) {
        glDeleteProgram(id_);
    }
}

GLProgram::GLProgram(GLProgram&& other) noexcept : id_(std::exchange(other.id_, 0)) {}

GLProgram& GLProgram::operator=(GLProgram&& other) noexcept {
    if (this != &other) {
        if (id_ != 0) {
            glDeleteProgram(id_);
        }
        id_ = std::exchange(other.id_, 0);
    }
    return *this;
}

void GLProgram::Use() const {
    detail::RequireValid(IsValid(), "use", "OpenGL program");
    FE_GL(glUseProgram(id_));
}

}  // namespace ferrum::renderer::opengl

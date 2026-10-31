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

void ValidateShaders(const GLShader& vertex_shader, const GLShader& fragment_shader) {
    detail::RequireValid(vertex_shader.IsValid(), "create OpenGL program", "vertex shader");
    detail::RequireValid(fragment_shader.IsValid(), "create OpenGL program", "fragment shader");
    FE_THROW_IF(vertex_shader.GetType() != ShaderType::Vertex ||
                    fragment_shader.GetType() != ShaderType::Fragment,
                "OpenGL program requires a vertex shader followed by a fragment shader");
}

GLuint LinkProgram(const GLShader& vertex_shader, const GLShader& fragment_shader) {
    ValidateShaders(vertex_shader, fragment_shader);
    GLuint id = 0;
    try {
        FE_GL(id = glCreateProgram());
        FE_THROW_IF(id == 0, "Failed to create OpenGL program");
        FE_GL(glAttachShader(id, vertex_shader.Id()));
        FE_GL(glAttachShader(id, fragment_shader.Id()));
        FE_GL(glLinkProgram(id));
        CheckProgramLinking(id);
        FE_GL(glDetachShader(id, vertex_shader.Id()));
        FE_GL(glDetachShader(id, fragment_shader.Id()));
        return id;
    } catch (...) {
        if (id != 0) {
            glDeleteProgram(id);
        }
        throw;
    }
}

}  // namespace

GLProgram::GLProgram(const GLShader& vertex_shader, const GLShader& fragment_shader)
    : id_(LinkProgram(vertex_shader, fragment_shader)) {}

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

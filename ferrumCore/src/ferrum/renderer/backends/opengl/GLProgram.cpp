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

struct OpenGLShaderPair {
    const GLShader* vertex = nullptr;
    const GLShader* fragment = nullptr;
};

OpenGLShaderPair GetOpenGLShaders(const renderer::ProgramModuleDesc& description) {
    const auto* vertex_shader = dynamic_cast<const GLShader*>(&description.vertex_shader);
    const auto* fragment_shader = dynamic_cast<const GLShader*>(&description.fragment_shader);
    FE_THROW_IF(vertex_shader == nullptr || fragment_shader == nullptr,
                "OpenGL program requires OpenGL shader modules");
    detail::RequireValid(vertex_shader->IsValid(), "create OpenGL program", "vertex shader");
    detail::RequireValid(fragment_shader->IsValid(), "create OpenGL program", "fragment shader");
    FE_THROW_IF(vertex_shader->GetType() != ShaderType::Vertex ||
                    fragment_shader->GetType() != ShaderType::Fragment,
                "OpenGL program requires a vertex shader followed by a fragment shader");
    return {.vertex = vertex_shader, .fragment = fragment_shader};
}

GLuint LinkProgram(const OpenGLShaderPair& shaders) {
    GLuint id = 0;
    try {
        FE_GL(id = glCreateProgram());
        FE_THROW_IF(id == 0, "Failed to create OpenGL program");
        FE_GL(glAttachShader(id, shaders.vertex->Id()));
        FE_GL(glAttachShader(id, shaders.fragment->Id()));
        FE_GL(glLinkProgram(id));
        CheckProgramLinking(id);
        FE_GL(glDetachShader(id, shaders.vertex->Id()));
        FE_GL(glDetachShader(id, shaders.fragment->Id()));
        return id;
    } catch (...) {
        if (id != 0) {
            glDeleteProgram(id);
        }
        throw;
    }
}

}  // namespace

GLProgram::GLProgram(const renderer::ProgramModuleDesc& description)
    : id_(LinkProgram(GetOpenGLShaders(description))) {}

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

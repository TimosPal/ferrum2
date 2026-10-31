#include "ferrum/renderer/backends/opengl/GLProgram.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <type_traits>
#include <utility>

#include "ferrum/common/exception.hpp"
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

struct UniformSetter {
    GLuint id;
    GLint location;

    void operator()(float parameter) const { FE_GL(glProgramUniform1f(id, location, parameter)); }

    void operator()(int parameter) const { FE_GL(glProgramUniform1i(id, location, parameter)); }

    void operator()(const glm::vec2& parameter) const {
        FE_GL(glProgramUniform2fv(id, location, 1, glm::value_ptr(parameter)));
    }

    void operator()(const glm::vec3& parameter) const {
        FE_GL(glProgramUniform3fv(id, location, 1, glm::value_ptr(parameter)));
    }

    void operator()(const glm::vec4& parameter) const {
        FE_GL(glProgramUniform4fv(id, location, 1, glm::value_ptr(parameter)));
    }

    void operator()(const glm::mat4& parameter) const {
        FE_GL(glProgramUniformMatrix4fv(id, location, 1, GL_FALSE, glm::value_ptr(parameter)));
    }
};

void SetUniform(GLuint id, GLint location, const renderer::ProgramParameterValue& value) {
    std::visit(
        UniformSetter{
            .id = id,
            .location = location,
        },
        value);
}

}  // namespace

GLProgram::GLProgram(const GLShader& vertex_shader, const GLShader& fragment_shader)
    : id_(LinkProgram(vertex_shader, fragment_shader)) {}

GLProgram::~GLProgram() {
    if (id_ != 0) {
        glDeleteProgram(id_);
    }
}

GLProgram::GLProgram(GLProgram&& other) noexcept
    : id_(std::exchange(other.id_, 0)),
      parameter_locations_(std::move(other.parameter_locations_)) {}

GLProgram& GLProgram::operator=(GLProgram&& other) noexcept {
    if (this != &other) {
        if (id_ != 0) {
            glDeleteProgram(id_);
        }
        id_ = std::exchange(other.id_, 0);
        parameter_locations_ = std::move(other.parameter_locations_);
    }
    return *this;
}

void GLProgram::Use() const {
    detail::RequireValid(IsValid(), "use", "OpenGL program");
    FE_GL(glUseProgram(id_));
}

void GLProgram::SetParameter(std::string_view name, const renderer::ProgramParameterValue& value) {
    detail::RequireValid(IsValid(), "set a parameter on", "OpenGL program");
    FE_THROW_IF(name.empty(), "OpenGL program parameter name cannot be empty");

    auto [location_entry, inserted] = parameter_locations_.try_emplace(std::string(name), -1);
    if (inserted) {
        FE_GL(location_entry->second = glGetUniformLocation(id_, location_entry->first.c_str()));
    }
    FE_THROW_IF(location_entry->second < 0, "OpenGL program has no active parameter named '{}'",
                location_entry->first);

    SetUniform(id_, location_entry->second, value);
}

}  // namespace ferrum::renderer::opengl

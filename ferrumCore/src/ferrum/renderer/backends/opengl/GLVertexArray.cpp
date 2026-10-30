#include "ferrum/renderer/backends/opengl/GLVertexArray.hpp"

#include <limits>
#include <utility>

#include "ferrum/renderer/backends/opengl/utils.hpp"

namespace ferrum::renderer::opengl {
namespace {

GLenum GetGLAttributeType(LayoutDescriptor::AttributeType type) {
    switch (type) {
        case LayoutDescriptor::AttributeType::Float:
        case LayoutDescriptor::AttributeType::Vec2:
        case LayoutDescriptor::AttributeType::Vec3:
        case LayoutDescriptor::AttributeType::Vec4:
            return GL_FLOAT;
        case LayoutDescriptor::AttributeType::Int:
        case LayoutDescriptor::AttributeType::IVec2:
        case LayoutDescriptor::AttributeType::IVec3:
        case LayoutDescriptor::AttributeType::IVec4:
            return GL_INT;
        case LayoutDescriptor::AttributeType::UInt:
        case LayoutDescriptor::AttributeType::UVec2:
        case LayoutDescriptor::AttributeType::UVec3:
        case LayoutDescriptor::AttributeType::UVec4:
            return GL_UNSIGNED_INT;
    }
    FE_THROW("Unsupported OpenGL vertex attribute type");
}

void ValidateVertexBinding(const LayoutDescriptor& layout, unsigned int binding_index) {
    FE_THROW_IF(!layout.IsValid() || layout.Stride() == 0,
                "OpenGL vertex layout must be valid and have a nonzero stride");
    FE_THROW_IF(layout.Stride() > static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()),
                "OpenGL vertex stride is too large");
    GLint max_bindings = 0;
    GLint max_stride = 0;
    GLint max_attributes = 0;
    GLint max_offset = 0;
    FE_GL(glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS, &max_bindings));
    FE_GL(glGetIntegerv(GL_MAX_VERTEX_ATTRIB_STRIDE, &max_stride));
    FE_GL(glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_attributes));
    FE_GL(glGetIntegerv(GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET, &max_offset));
    FE_THROW_IF(std::cmp_greater_equal(binding_index, max_bindings),
                "OpenGL vertex binding index exceeds the device limit");
    FE_THROW_IF(layout.Stride() > static_cast<std::size_t>(max_stride),
                "OpenGL vertex stride exceeds the device limit");
    for (const auto& metadata : layout.Metadata()) {
        FE_THROW_IF(std::cmp_greater_equal(metadata.attribute.location, max_attributes),
                    "OpenGL vertex attribute location exceeds the device limit");
        FE_THROW_IF(std::cmp_greater(metadata.offset, max_offset),
                    "OpenGL vertex attribute offset exceeds the device limit");
    }
}

void ConfigureVertexAttribute(GLuint vertex_array_id,
                              const LayoutDescriptor::AttributeMetadata& metadata,
                              GLuint binding_index) {
    const GLenum type = GetGLAttributeType(metadata.attribute.type);
    const auto offset = static_cast<GLuint>(metadata.offset);
    if (metadata.is_integer) {
        FE_GL(glVertexArrayAttribIFormat(vertex_array_id, metadata.attribute.location,
                                         metadata.components, type, offset));
    } else {
        FE_GL(glVertexArrayAttribFormat(vertex_array_id, metadata.attribute.location,
                                        metadata.components, type, GL_FALSE, offset));
    }
    FE_GL(glVertexArrayAttribBinding(vertex_array_id, metadata.attribute.location, binding_index));
    FE_GL(glEnableVertexArrayAttrib(vertex_array_id, metadata.attribute.location));
}

}  // namespace

GLVertexArray::GLVertexArray() : GLVertexArray(nullptr) {
    FE_GL(glCreateVertexArrays(1, &id_));
    FE_THROW_IF(!IsValid(), "Failed to create OpenGL vertex array");
}

GLVertexArray::~GLVertexArray() {
    if (id_ != 0) {
        glDeleteVertexArrays(1, &id_);
    }
}

GLVertexArray::GLVertexArray(GLVertexArray&& other) noexcept : id_(std::exchange(other.id_, 0)) {}

GLVertexArray& GLVertexArray::operator=(GLVertexArray&& other) noexcept {
    if (this != &other) {
        if (id_ != 0) {
            glDeleteVertexArrays(1, &id_);
        }
        id_ = std::exchange(other.id_, 0);
    }
    return *this;
}

void GLVertexArray::SetVertexBuffer(const GLBuffer& buffer, const LayoutDescriptor& layout,
                                    unsigned int binding_index) {
    detail::RequireValid(IsValid(), "configure vertex buffer", "OpenGL vertex array");
    detail::RequireValid(buffer.IsValid(), "configure vertex buffer", "OpenGL buffer");
    ValidateVertexBinding(layout, binding_index);
    FE_GL(glVertexArrayVertexBuffer(id_, binding_index, buffer.Id(), 0,
                                    static_cast<GLsizei>(layout.Stride())));

    for (const auto& metadata : layout.Metadata()) {
        ConfigureVertexAttribute(id_, metadata, binding_index);
    }
}

void GLVertexArray::SetIndexBuffer(const GLBuffer& buffer) {
    detail::RequireValid(IsValid(), "configure index buffer", "OpenGL vertex array");
    detail::RequireValid(buffer.IsValid(), "configure index buffer", "OpenGL buffer");
    FE_GL(glVertexArrayElementBuffer(id_, buffer.Id()));
}

void GLVertexArray::Bind() const {
    detail::RequireValid(IsValid(), "bind", "OpenGL vertex array");
    FE_GL(glBindVertexArray(id_));
}

}  // namespace ferrum::renderer::opengl

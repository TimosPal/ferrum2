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
    FE_THROW_IF(layout.Stride() > static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()),
                "OpenGL vertex stride is too large");
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

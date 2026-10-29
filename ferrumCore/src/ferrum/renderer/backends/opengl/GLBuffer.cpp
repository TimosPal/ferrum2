#include "ferrum/renderer/backends/opengl/GLBuffer.hpp"

#include <limits>
#include <utility>

#include "ferrum/renderer/backends/opengl/utils.hpp"

namespace ferrum::renderer::opengl {
namespace {

GLenum BufferUsage(GLBuffer::Usage usage) {
    switch (usage) {
        case GLBuffer::Usage::StaticDraw:
            return GL_STATIC_DRAW;
        case GLBuffer::Usage::DynamicDraw:
            return GL_DYNAMIC_DRAW;
        case GLBuffer::Usage::StreamDraw:
            return GL_STREAM_DRAW;
    }
    FE_THROW("Unsupported OpenGL buffer usage");
}

}  // namespace

GLBuffer::GLBuffer() : GLBuffer(nullptr) {
    FE_GL(glCreateBuffers(1, &id_));
    FE_THROW_IF(!IsValid(), "Failed to create OpenGL buffer");
}

GLBuffer::~GLBuffer() {
    if (id_ != 0) {
        glDeleteBuffers(1, &id_);
    }
}

GLBuffer::GLBuffer(GLBuffer&& other) noexcept : id_(std::exchange(other.id_, 0)) {}

GLBuffer& GLBuffer::operator=(GLBuffer&& other) noexcept {
    if (this != &other) {
        if (id_ != 0) {
            glDeleteBuffers(1, &id_);
        }
        id_ = std::exchange(other.id_, 0);
    }
    return *this;
}

void GLBuffer::Upload(std::span<const std::byte> data, Usage usage) {
    detail::RequireValid(IsValid(), "upload data", "OpenGL buffer");
    FE_THROW_IF(data.size() > static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max()),
                "OpenGL buffer data is too large");
    FE_GL(glNamedBufferData(id_, static_cast<GLsizeiptr>(data.size()), data.data(),
                            BufferUsage(usage)));
}

}  // namespace ferrum::renderer::opengl

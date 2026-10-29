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

GLBuffer::GLBuffer(GLBuffer&& other) noexcept
    : id_(std::exchange(other.id_, 0)), size_(std::exchange(other.size_, 0)) {}

GLBuffer& GLBuffer::operator=(GLBuffer&& other) noexcept {
    if (this != &other) {
        if (id_ != 0) {
            glDeleteBuffers(1, &id_);
        }
        id_ = std::exchange(other.id_, 0);
        size_ = std::exchange(other.size_, 0);
    }
    return *this;
}

void GLBuffer::Upload(std::span<const std::byte> data, Usage usage) {
    detail::RequireValid(IsValid(), "upload data", "OpenGL buffer");
    FE_THROW_IF(data.size() > static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max()),
                "OpenGL buffer data is too large");
    FE_GL(glNamedBufferData(id_, static_cast<GLsizeiptr>(data.size()), data.data(),
                            BufferUsage(usage)));
    size_ = data.size();
}

void GLBuffer::Allocate(std::size_t size, Usage usage) {
    detail::RequireValid(IsValid(), "allocate storage", "OpenGL buffer");
    FE_THROW_IF(size > static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max()),
                "OpenGL buffer size is too large");
    FE_GL(glNamedBufferData(id_, static_cast<GLsizeiptr>(size), nullptr, BufferUsage(usage)));
    size_ = size;
}

void GLBuffer::Update(std::size_t offset, std::span<const std::byte> data) {
    detail::RequireValid(IsValid(), "update data", "OpenGL buffer");
    FE_THROW_IF(offset > size_ || data.size() > size_ - offset,
                "OpenGL buffer update range exceeds its allocated size");
    FE_THROW_IF(offset > static_cast<std::size_t>(std::numeric_limits<GLintptr>::max()),
                "OpenGL buffer update offset is too large");
    FE_THROW_IF(data.size() > static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max()),
                "OpenGL buffer update data is too large");
    if (data.empty()) {
        return;
    }

    FE_GL(glNamedBufferSubData(id_, static_cast<GLintptr>(offset),
                               static_cast<GLsizeiptr>(data.size()), data.data()));
}

}  // namespace ferrum::renderer::opengl

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>
#include <utility>

#include "ferrum/renderer/backends/opengl/GLBuffer.hpp"
#include "ferrum/renderer/backends/opengl/LayoutDescriptor.hpp"

namespace ferrum::renderer::opengl {

class GLMeshVertexBufferConfig {
public:
    template <typename Vertex>
    GLMeshVertexBufferConfig(std::span<const Vertex> vertices, LayoutDescriptor layout,
                             GLBuffer::Usage usage = GLBuffer::Usage::StaticDraw)
        : data_(std::as_bytes(vertices)),
          count_(vertices.size()),
          layout_(std::move(layout)),
          usage_(usage) {
        static_assert(std::is_trivially_copyable_v<Vertex>,
                      "Vertex data must be trivially copyable");
    }

    GLMeshVertexBufferConfig(std::size_t vertexCount, LayoutDescriptor layout,
                             GLBuffer::Usage usage = GLBuffer::Usage::StaticDraw)
        : count_(vertexCount), layout_(std::move(layout)), usage_(usage) {}

    std::span<const std::byte> Data() const noexcept { return data_; }
    std::size_t Count() const noexcept { return count_; }
    const LayoutDescriptor& GetLayout() const noexcept { return layout_; }
    GLBuffer::Usage GetUsage() const noexcept { return usage_; }
    bool HasData() const noexcept { return !data_.empty(); }

private:
    std::span<const std::byte> data_;
    std::size_t count_;
    LayoutDescriptor layout_;
    GLBuffer::Usage usage_;
};

class GLMeshIndexBufferConfig {
public:
    enum class Type { UInt8, UInt16, UInt32 };

    GLMeshIndexBufferConfig(std::span<const std::uint8_t> indices,
                            GLBuffer::Usage usage = GLBuffer::Usage::StaticDraw)
        : data_(std::as_bytes(indices)),
          count_(indices.size()),
          type_(Type::UInt8),
          usage_(usage) {}
    GLMeshIndexBufferConfig(std::span<const std::uint16_t> indices,
                            GLBuffer::Usage usage = GLBuffer::Usage::StaticDraw)
        : data_(std::as_bytes(indices)),
          count_(indices.size()),
          type_(Type::UInt16),
          usage_(usage) {}
    GLMeshIndexBufferConfig(std::span<const std::uint32_t> indices,
                            GLBuffer::Usage usage = GLBuffer::Usage::StaticDraw)
        : data_(std::as_bytes(indices)),
          count_(indices.size()),
          type_(Type::UInt32),
          usage_(usage) {}
    GLMeshIndexBufferConfig(std::size_t indexCount, Type type,
                            GLBuffer::Usage usage = GLBuffer::Usage::StaticDraw)
        : count_(indexCount), type_(type), usage_(usage) {}

    std::span<const std::byte> Data() const noexcept { return data_; }
    std::size_t Count() const noexcept { return count_; }
    Type GetType() const noexcept { return type_; }
    GLBuffer::Usage GetUsage() const noexcept { return usage_; }
    bool HasData() const noexcept { return !data_.empty(); }

private:
    std::span<const std::byte> data_;
    std::size_t count_;
    Type type_;
    GLBuffer::Usage usage_;
};

}  // namespace ferrum::renderer::opengl

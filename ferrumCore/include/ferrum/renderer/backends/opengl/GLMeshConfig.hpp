#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>
#include <utility>

#include "ferrum/renderer/LayoutDescriptor.hpp"
#include "ferrum/renderer/backends/opengl/GLBuffer.hpp"

namespace ferrum::renderer::opengl {

class GLMeshVertexBufferConfig {
public:
    template <typename Vertex>
    GLMeshVertexBufferConfig(std::span<const Vertex> vertices, LayoutDescriptor layout,
                             GLBuffer::Usage usage = GLBuffer::Usage::StaticDraw)
        : data_(std::as_bytes(vertices)),
          vertex_capacity_(vertices.size()),
          vertex_count_(vertices.size()),
          layout_(std::move(layout)),
          usage_(usage) {
        static_assert(std::is_trivially_copyable_v<Vertex>,
                      "Vertex data must be trivially copyable");
    }

    GLMeshVertexBufferConfig(std::size_t vertex_count, LayoutDescriptor layout,
                             GLBuffer::Usage usage = GLBuffer::Usage::StaticDraw)
        : vertex_capacity_(vertex_count), layout_(std::move(layout)), usage_(usage) {}

    std::span<const std::byte> Data() const noexcept { return data_; }
    std::size_t VertexCapacity() const noexcept { return vertex_capacity_; }
    std::size_t VertexCount() const noexcept { return vertex_count_; }
    std::size_t GetBytesSize() const noexcept { return data_.size(); }
    std::size_t GetCapacityBytesSize() const noexcept {
        return vertex_capacity_ * layout_.Stride();
    }
    const LayoutDescriptor& GetLayout() const noexcept { return layout_; }
    GLBuffer::Usage GetUsage() const noexcept { return usage_; }
    bool HasData() const noexcept { return !data_.empty(); }

private:
    std::span<const std::byte> data_;
    std::size_t vertex_capacity_ = 0;
    std::size_t vertex_count_ = 0;
    LayoutDescriptor layout_;
    GLBuffer::Usage usage_ = GLBuffer::Usage::StaticDraw;
};

class GLMeshIndexBufferConfig {
public:
    enum class Type { UInt8, UInt16, UInt32 };

    GLMeshIndexBufferConfig(std::span<const std::uint8_t> indices,
                            GLBuffer::Usage usage = GLBuffer::Usage::StaticDraw)
        : data_(std::as_bytes(indices)),
          index_count_(indices.size()),
          index_capacity_(indices.size()),
          type_(Type::UInt8),
          usage_(usage) {}
    GLMeshIndexBufferConfig(std::span<const std::uint16_t> indices,
                            GLBuffer::Usage usage = GLBuffer::Usage::StaticDraw)
        : data_(std::as_bytes(indices)),
          index_count_(indices.size()),
          index_capacity_(indices.size()),
          type_(Type::UInt16),
          usage_(usage) {}
    GLMeshIndexBufferConfig(std::span<const std::uint32_t> indices,
                            GLBuffer::Usage usage = GLBuffer::Usage::StaticDraw)
        : data_(std::as_bytes(indices)),
          index_count_(indices.size()),
          index_capacity_(indices.size()),
          type_(Type::UInt32),
          usage_(usage) {}
    GLMeshIndexBufferConfig(std::size_t index_count, Type type,
                            GLBuffer::Usage usage = GLBuffer::Usage::StaticDraw)
        : index_capacity_(index_count), type_(type), usage_(usage) {}

    std::span<const std::byte> Data() const noexcept { return data_; }
    std::size_t IndexCapacity() const noexcept { return index_capacity_; }
    std::size_t IndexCount() const noexcept { return index_count_; }
    std::size_t GetBytesSize() const noexcept { return data_.size(); }
    std::size_t GetCapacityBytesSize() const noexcept {
        return index_capacity_ * (type_ == Type::UInt8    ? sizeof(std::uint8_t)
                                  : type_ == Type::UInt16 ? sizeof(std::uint16_t)
                                                          : sizeof(std::uint32_t));
    }
    Type GetIndexType() const noexcept { return type_; }
    GLBuffer::Usage GetUsage() const noexcept { return usage_; }
    bool HasData() const noexcept { return !data_.empty(); }

private:
    std::span<const std::byte> data_;
    std::size_t index_capacity_ = 0;
    std::size_t index_count_ = 0;
    Type type_;
    GLBuffer::Usage usage_;
};

}  // namespace ferrum::renderer::opengl

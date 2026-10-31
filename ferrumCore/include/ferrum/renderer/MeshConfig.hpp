#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>
#include <utility>

#include "ferrum/renderer/LayoutDescriptor.hpp"

namespace ferrum::renderer {

enum class IndexType {
    UInt8,
    UInt16,
    UInt32,
};

class VertexBufferConfig {
public:
    virtual ~VertexBufferConfig() = default;
    VertexBufferConfig(const VertexBufferConfig&) = default;
    VertexBufferConfig& operator=(const VertexBufferConfig&) = default;
    VertexBufferConfig(VertexBufferConfig&&) = default;
    VertexBufferConfig& operator=(VertexBufferConfig&&) = default;

    template <typename Vertex>
    VertexBufferConfig(std::span<const Vertex> vertices, LayoutDescriptor layout)
        : data_(std::as_bytes(vertices)),
          vertex_capacity_(vertices.size()),
          vertex_count_(vertices.size()),
          layout_(std::move(layout)) {
        static_assert(std::is_trivially_copyable_v<Vertex>,
                      "Vertex data must be trivially copyable");
    }

    VertexBufferConfig(std::size_t vertex_count, LayoutDescriptor layout)
        : vertex_capacity_(vertex_count), layout_(std::move(layout)) {}

    std::span<const std::byte> Data() const noexcept { return data_; }
    std::size_t VertexCapacity() const noexcept { return vertex_capacity_; }
    std::size_t VertexCount() const noexcept { return vertex_count_; }
    std::size_t GetBytesSize() const noexcept { return data_.size(); }
    std::size_t GetCapacityBytesSize() const noexcept {
        return vertex_capacity_ * layout_.Stride();
    }
    const LayoutDescriptor& GetLayout() const noexcept { return layout_; }
    bool HasData() const noexcept { return !data_.empty(); }

private:
    std::span<const std::byte> data_;
    std::size_t vertex_capacity_ = 0;
    std::size_t vertex_count_ = 0;
    LayoutDescriptor layout_;
};

class IndexBufferConfig {
public:
    using Type = IndexType;

    virtual ~IndexBufferConfig() = default;
    IndexBufferConfig(const IndexBufferConfig&) = default;
    IndexBufferConfig& operator=(const IndexBufferConfig&) = default;
    IndexBufferConfig(IndexBufferConfig&&) = default;
    IndexBufferConfig& operator=(IndexBufferConfig&&) = default;

    template <typename Index>
    IndexBufferConfig(std::span<const Index> indices)
        : data_(std::as_bytes(indices)),
          index_count_(indices.size()),
          index_capacity_(indices.size()),
          type_(GetType<Index>()) {
        static_assert(std::is_same_v<Index, std::uint8_t> || std::is_same_v<Index, std::uint16_t> ||
                          std::is_same_v<Index, std::uint32_t>,
                      "Index type must be uint8_t, uint16_t, or uint32_t");
    }
    IndexBufferConfig(std::size_t index_count, Type type)
        : index_capacity_(index_count), type_(type) {}

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
    bool HasData() const noexcept { return !data_.empty(); }

private:
    template <typename Index>
    static constexpr Type GetType() {
        if constexpr (std::is_same_v<Index, std::uint8_t>) {
            return Type::UInt8;
        } else if constexpr (std::is_same_v<Index, std::uint16_t>) {
            return Type::UInt16;
        } else {
            return Type::UInt32;
        }
    }

    std::span<const std::byte> data_;
    std::size_t index_capacity_ = 0;
    std::size_t index_count_ = 0;
    Type type_;
};

}  // namespace ferrum::renderer

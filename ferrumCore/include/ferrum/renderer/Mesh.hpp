#pragma once

#include <span>

#include "ferrum/renderer/MeshConfig.hpp"

namespace ferrum::renderer {

enum class BufferUsage {
    Static,
    Dynamic,
    Stream,
};

enum class PrimitiveTopology {
    Triangles,
    Lines,
    Points,
};

struct MeshDesc {
    std::span<const VertexBufferConfig> vertex_buffers;
    const IndexBufferConfig& index_buffer;
    BufferUsage usage = BufferUsage::Static;

    MeshDesc(std::span<const VertexBufferConfig> vertex_buffers,
             const IndexBufferConfig& index_buffer, BufferUsage usage = BufferUsage::Static)
        : vertex_buffers(vertex_buffers), index_buffer(index_buffer), usage(usage) {}

    MeshDesc(const VertexBufferConfig& vertex_buffer, const IndexBufferConfig& index_buffer,
             BufferUsage usage = BufferUsage::Static)
        : MeshDesc(std::span<const VertexBufferConfig>(&vertex_buffer, 1), index_buffer, usage) {}
};

class Mesh {
public:
    virtual ~Mesh() = default;
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&&) noexcept = default;
    Mesh& operator=(Mesh&&) noexcept = default;

protected:
    Mesh() = default;
};

}  // namespace ferrum::renderer

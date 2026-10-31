#pragma once

#include "ferrum/renderer/MeshConfig.hpp"
#include "ferrum/renderer/backends/opengl/GLBuffer.hpp"

namespace ferrum::renderer::opengl {

class GLMeshVertexBufferConfig : public renderer::VertexBufferConfig {
public:
    explicit GLMeshVertexBufferConfig(const renderer::VertexBufferConfig& config,
                                      GLBuffer::Usage usage = GLBuffer::Usage::StaticDraw)
        : renderer::VertexBufferConfig(config), usage_(usage) {}

    template <typename Vertex>
    GLMeshVertexBufferConfig(std::span<const Vertex> vertices, LayoutDescriptor layout,
                             GLBuffer::Usage usage = GLBuffer::Usage::StaticDraw)
        : renderer::VertexBufferConfig(vertices, std::move(layout)), usage_(usage) {}

    GLMeshVertexBufferConfig(std::size_t vertex_count, LayoutDescriptor layout,
                             GLBuffer::Usage usage = GLBuffer::Usage::StaticDraw)
        : renderer::VertexBufferConfig(vertex_count, std::move(layout)), usage_(usage) {}

    GLBuffer::Usage GetUsage() const noexcept { return usage_; }

private:
    GLBuffer::Usage usage_ = GLBuffer::Usage::StaticDraw;
};

class GLMeshIndexBufferConfig : public renderer::IndexBufferConfig {
public:
    using Type = renderer::IndexType;

    explicit GLMeshIndexBufferConfig(const renderer::IndexBufferConfig& config,
                                     GLBuffer::Usage usage = GLBuffer::Usage::StaticDraw)
        : renderer::IndexBufferConfig(config), usage_(usage) {}

    template <typename Index>
    GLMeshIndexBufferConfig(std::span<const Index> indices,
                            GLBuffer::Usage usage = GLBuffer::Usage::StaticDraw)
        : renderer::IndexBufferConfig(indices), usage_(usage) {}
    GLMeshIndexBufferConfig(std::size_t index_count, Type type,
                            GLBuffer::Usage usage = GLBuffer::Usage::StaticDraw)
        : renderer::IndexBufferConfig(index_count, type), usage_(usage) {}

    GLBuffer::Usage GetUsage() const noexcept { return usage_; }

private:
    GLBuffer::Usage usage_ = GLBuffer::Usage::StaticDraw;
};

}  // namespace ferrum::renderer::opengl

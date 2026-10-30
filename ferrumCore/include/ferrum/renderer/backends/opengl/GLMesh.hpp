#pragma once

#include <cstddef>
#include <span>
#include <vector>

#include "ferrum/renderer/backends/opengl/GLBuffer.hpp"
#include "ferrum/renderer/backends/opengl/GLMeshConfig.hpp"
#include "ferrum/renderer/backends/opengl/GLVertexArray.hpp"

namespace ferrum::renderer::opengl {

class GLMesh {
public:
    GLMesh(std::span<const GLMeshVertexBufferConfig> vertex_buffers,
           const GLMeshIndexBufferConfig& indices);
    ~GLMesh() = default;
    GLMesh(const GLMesh&) = delete;
    GLMesh& operator=(const GLMesh&) = delete;
    GLMesh(GLMesh&& other) noexcept = default;
    GLMesh& operator=(GLMesh&& other) noexcept = default;

    // Index values must reference initialized vertices in every binding.
    void Draw();
    void Draw(std::size_t index_count, std::size_t index_offset);

    // Resizing discards contents and resets the corresponding active count.
    void ResizeVertexBuffer(unsigned int buffer_index, std::size_t new_vertex_capacity);
    void ResizeIndexBuffer(std::size_t new_index_capacity);
    // Updates leave counts unchanged. Offsets are measured in vertices or indices, not bytes.
    void UpdateVertexBuffer(unsigned int buffer_index, std::span<const std::byte> data,
                            std::size_t vertex_offset);
    void UpdateIndexBuffer(std::span<const std::byte> data, std::size_t index_offset);
    // Counts may expose only initialized data; every vertex binding must have capacity.
    void SetVertexCount(std::size_t vertex_count);
    void SetIndexCount(std::size_t index_count);

private:
    struct VertexBufferInfo {
        LayoutDescriptor layout;
        GLBuffer buffer;
        GLBuffer::Usage usage;
    };

    struct IndexBufferInfo {
        unsigned int index_type = 0;
        size_t index_size_bytes = 0;
        size_t index_count = 0;
        GLBuffer buffer;
        GLBuffer::Usage usage;
    };

    void ValidateVertexBufferConfig(const GLMeshVertexBufferConfig& vb_config) const;
    void ValidateDrawRange(std::size_t index_count, std::size_t index_offset) const;
    void ValidateVertexCount(std::size_t vertex_count) const;
    void BindAndUploadVertexBuffers(std::span<const GLMeshVertexBufferConfig> vertex_buffers);
    void BindAndUploadIndexBuffer(const GLMeshIndexBufferConfig& indices);

    std::vector<VertexBufferInfo> vertex_buffers_;
    std::size_t vertex_count_ = 0;
    IndexBufferInfo index_buffer_;
    GLVertexArray vertex_array_;
};

}  // namespace ferrum::renderer::opengl

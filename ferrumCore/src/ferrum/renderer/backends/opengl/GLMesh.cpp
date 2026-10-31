#include "ferrum/renderer/backends/opengl/GLMesh.hpp"

#include <glad/gl.h>

#include <limits>
#include <unordered_set>

#include "ferrum/common/exception.hpp"
#include "ferrum/renderer/backends/opengl/utils.hpp"

namespace ferrum::renderer::opengl {

namespace {

void CheckByteSize(std::size_t count, std::size_t element_size) {
    FE_THROW_IF(
        count > static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max()) / element_size,
        "Mesh buffer byte size exceeds the OpenGL size limit");
}

template <typename Buffer>
void UploadOrAllocateBuffer(Buffer& buffer, const auto& config) {
    if (config.HasData()) {
        buffer.Upload(config.Data(), config.GetUsage());
    } else {
        buffer.Allocate(config.GetCapacityBytesSize(), config.GetUsage());
    }
}

unsigned int ToGLIndexType(GLMeshIndexBufferConfig::Type type) {
    switch (type) {
        case GLMeshIndexBufferConfig::Type::UInt16:
            return GL_UNSIGNED_SHORT;
        case GLMeshIndexBufferConfig::Type::UInt32:
            return GL_UNSIGNED_INT;
        case GLMeshIndexBufferConfig::Type::UInt8:
            return GL_UNSIGNED_BYTE;
        default:
            FE_THROW("Unsupported index buffer type");
    }
}

std::size_t GetIndexSizeBytes(GLMeshIndexBufferConfig::Type type) {
    switch (type) {
        case GLMeshIndexBufferConfig::Type::UInt16:
            return 2;
        case GLMeshIndexBufferConfig::Type::UInt32:
            return 4;
        case GLMeshIndexBufferConfig::Type::UInt8:
            return 1;
        default:
            FE_THROW("Unsupported index buffer type");
    }
}

void ValidateIndexBufferConfig(const GLMeshIndexBufferConfig& indices) {
    const auto index_size = GetIndexSizeBytes(indices.GetIndexType());
    CheckByteSize(indices.IndexCapacity(), index_size);
    FE_THROW_IF(indices.IndexCount() > indices.IndexCapacity(),
                "Index count exceeds index buffer capacity");
    FE_THROW_IF(indices.Data().size() % index_size != 0 ||
                    indices.Data().size() / index_size != indices.IndexCount(),
                "Index data size does not match the configured index count");
}

}  // namespace

void GLMesh::BindAndUploadIndexBuffer(const GLMeshIndexBufferConfig& indices) {
    ValidateIndexBufferConfig(indices);
    UploadOrAllocateBuffer(index_buffer_.buffer, indices);
    index_buffer_.index_type = ToGLIndexType(indices.GetIndexType());
    index_buffer_.index_size_bytes = GetIndexSizeBytes(indices.GetIndexType());
    index_buffer_.index_count = indices.IndexCount();
    index_buffer_.usage = indices.GetUsage();
    vertex_array_.SetIndexBuffer(index_buffer_.buffer);
}

void GLMesh::ValidateVertexBufferConfig(const GLMeshVertexBufferConfig& vb_config) const {
    const auto stride = vb_config.GetLayout().Stride();
    FE_THROW_IF(stride == 0 || !vb_config.GetLayout().IsValid(),
                "Mesh vertex layout must be valid and have a nonzero stride");
    CheckByteSize(vb_config.VertexCapacity(), stride);
    FE_THROW_IF(vb_config.VertexCount() > vb_config.VertexCapacity(),
                "Vertex count exceeds vertex buffer capacity");
    FE_THROW_IF(vb_config.Data().size() % stride != 0 ||
                    vb_config.Data().size() / stride != vb_config.VertexCount(),
                "Vertex data size does not match the configured vertex count");
    FE_THROW_IF(!vertex_buffers_.empty() && vb_config.VertexCount() != vertex_count_,
                "Vertex bindings must have the same active vertex count");
    std::unordered_set<unsigned int> incoming_locations;
    for (const auto& incoming : vb_config.GetLayout().Metadata()) {
        incoming_locations.insert(incoming.attribute.location);
    }
    for (const auto& existing : vertex_buffers_) {
        for (const auto& attribute : existing.layout.Metadata()) {
            FE_THROW_IF(incoming_locations.contains(attribute.attribute.location),
                        "Vertex attribute location {} is used by multiple bindings",
                        attribute.attribute.location);
        }
    }
}

void GLMesh::BindAndUploadVertexBuffers(std::span<const GLMeshVertexBufferConfig> vertex_buffers) {
    unsigned int binding_index = 0;
    for (const auto& vb_config : vertex_buffers) {
        ValidateVertexBufferConfig(vb_config);
        GLBuffer vertex_buffer;
        UploadOrAllocateBuffer(vertex_buffer, vb_config);
        vertex_array_.SetVertexBuffer(vertex_buffer, vb_config.GetLayout(), binding_index);
        binding_index++;
        vertex_count_ = vb_config.VertexCount();
        vertex_buffers_.push_back({
            .layout = vb_config.GetLayout(),
            .buffer = std::move(vertex_buffer),
            .usage = vb_config.GetUsage(),
        });
    }
}

GLMesh::GLMesh(std::span<const GLMeshVertexBufferConfig> vertex_buffers,
               const GLMeshIndexBufferConfig& indices) {
    BindAndUploadVertexBuffers(vertex_buffers);
    BindAndUploadIndexBuffer(indices);
}

GLMesh::GLMesh(const GLMeshVertexBufferConfig& vertex_buffer,
               const GLMeshIndexBufferConfig& indices)
    : GLMesh(std::span<const GLMeshVertexBufferConfig>(&vertex_buffer, 1), indices) {}

void GLMesh::ValidateDrawRange(std::size_t index_count, std::size_t index_offset) const {
    FE_THROW_IF(index_offset > index_buffer_.index_count ||
                    index_count > index_buffer_.index_count - index_offset,
                "Draw range exceeds the active index count");
    FE_THROW_IF(index_count > static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()),
                "Draw index count exceeds the OpenGL count limit");
    FE_THROW_IF(index_count != 0 && !vertex_buffers_.empty() && vertex_count_ == 0,
                "Cannot draw vertex bindings with no active vertices");
    CheckByteSize(index_offset, index_buffer_.index_size_bytes);
}

void GLMesh::ResizeVertexBuffer(unsigned int buffer_index, std::size_t new_vertex_capacity) {
    GLMesh::VertexBufferInfo& vb_info = vertex_buffers_.at(buffer_index);
    CheckByteSize(new_vertex_capacity, vb_info.layout.Stride());
    vb_info.buffer.Allocate(new_vertex_capacity * vb_info.layout.Stride(), vb_info.usage);
    vertex_count_ = 0;
}

void GLMesh::ResizeIndexBuffer(std::size_t new_index_capacity) {
    CheckByteSize(new_index_capacity, index_buffer_.index_size_bytes);
    index_buffer_.buffer.Allocate(new_index_capacity * index_buffer_.index_size_bytes,
                                  index_buffer_.usage);
    index_buffer_.index_count = 0;
}

void GLMesh::UpdateVertexBuffer(unsigned int buffer_index, std::span<const std::byte> data,
                                std::size_t vertex_offset) {
    GLMesh::VertexBufferInfo& vb_info = vertex_buffers_.at(buffer_index);
    FE_THROW_IF(data.size() % vb_info.layout.Stride() != 0,
                "Vertex update must contain whole vertices");
    CheckByteSize(vertex_offset, vb_info.layout.Stride());
    vb_info.buffer.Update(vertex_offset * vb_info.layout.Stride(), data);
}

void GLMesh::UpdateIndexBuffer(std::span<const std::byte> data, std::size_t index_offset) {
    FE_THROW_IF(data.size() % index_buffer_.index_size_bytes != 0,
                "Index update must contain whole indices");
    CheckByteSize(index_offset, index_buffer_.index_size_bytes);
    index_buffer_.buffer.Update(index_offset * index_buffer_.index_size_bytes, data);
}

void GLMesh::ValidateVertexCount(std::size_t vertex_count) const {
    for (const auto& vb_info : vertex_buffers_) {
        FE_THROW_IF(vertex_count > vb_info.buffer.BytesSize() / vb_info.layout.Stride(),
                    "Vertex count exceeds a vertex binding's capacity");
    }
}

void GLMesh::SetVertexCount(std::size_t vertex_count) {
    ValidateVertexCount(vertex_count);
    vertex_count_ = vertex_count;
}

void GLMesh::SetIndexCount(std::size_t index_count) {
    FE_THROW_IF(index_count > index_buffer_.buffer.BytesSize() / index_buffer_.index_size_bytes,
                "Index count exceeds index buffer capacity");
    index_buffer_.index_count = index_count;
}

}  // namespace ferrum::renderer::opengl

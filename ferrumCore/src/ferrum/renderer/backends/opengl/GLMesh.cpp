#include "ferrum/renderer/backends/opengl/GLMesh.hpp"

#include <glad/gl.h>

#include <cstring>
#include <limits>
#include <unordered_set>
#include <utility>
#include <vector>

#include "ferrum/renderer/backends/opengl/utils.hpp"

namespace ferrum::renderer::opengl {

namespace {

std::size_t GetBufferSize(std::size_t count, std::size_t elementSize) {
    FE_THROW_IF(elementSize == 0, "OpenGL buffer element size cannot be zero");
    FE_THROW_IF(count > std::numeric_limits<std::size_t>::max() / elementSize,
                "OpenGL buffer size is too large");
    const auto size = count * elementSize;
    FE_THROW_IF(size > static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max()),
                "OpenGL buffer size is too large");
    return size;
}

void ValidateMeshSizes(std::span<const GLMeshVertexBufferConfig> vertexBuffers,
                       const GLMeshIndexBufferConfig& indices) {
    FE_THROW_IF(vertexBuffers.empty(), "OpenGL mesh requires at least one vertex buffer");
    FE_THROW_IF(indices.Count() > static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()),
                "OpenGL mesh has too many indices");
    FE_THROW_IF(vertexBuffers.size() > static_cast<std::size_t>(std::numeric_limits<GLuint>::max()),
                "OpenGL mesh has too many vertex buffers");
}

std::size_t GetVertexCount(const GLMeshVertexBufferConfig& vertexBuffer) {
    const auto stride = vertexBuffer.GetLayout().Stride();
    FE_THROW_IF(!vertexBuffer.HasData(), "OpenGL mesh vertex buffers require data");
    FE_THROW_IF(vertexBuffer.Count() == 0, "OpenGL mesh vertex buffers cannot be empty");
    FE_THROW_IF(stride == 0, "OpenGL mesh vertex buffer layout stride cannot be zero");
    const auto data = vertexBuffer.Data();
    FE_THROW_IF(data.size() % stride != 0,
                "OpenGL mesh vertex buffer size must be a multiple of its layout stride");
    FE_THROW_IF(data.size() / stride != vertexBuffer.Count(),
                "OpenGL mesh vertex buffer size does not match its vertex count");
    return vertexBuffer.Count();
}

void ValidateVertexAttributeLocations(std::span<const GLMeshVertexBufferConfig> vertexBuffers) {
    std::unordered_set<unsigned int> attributeLocations;
    for (const auto& vertexBuffer : vertexBuffers) {
        for (const auto& metadata : vertexBuffer.GetLayout().Metadata()) {
            const auto location = metadata.attribute.location;
            FE_THROW_IF(!attributeLocations.insert(location).second,
                        "Duplicate vertex attribute location {}", location);
        }
    }
}

std::size_t ValidateAndGetVertexCount(std::span<const GLMeshVertexBufferConfig> vertexBuffers) {
    const auto vertexCount = GetVertexCount(vertexBuffers.front());
    for (const auto& vertexBuffer : vertexBuffers.subspan(1)) {
        FE_THROW_IF(GetVertexCount(vertexBuffer) != vertexCount,
                    "OpenGL mesh vertex buffers must have matching vertex counts");
    }
    ValidateVertexAttributeLocations(vertexBuffers);
    return vertexCount;
}

template <typename Index>
void ValidateTypedIndices(const GLMeshIndexBufferConfig& indices, std::size_t vertexCount) {
    const auto data = indices.Data();
    for (std::size_t i = 0; i < indices.Count(); ++i) {
        Index index;
        std::memcpy(&index, data.data() + (i * sizeof(Index)), sizeof(Index));
        FE_THROW_IF(index >= vertexCount,
                    "OpenGL mesh index {} references a vertex outside the vertex buffers", i);
    }
}

void ValidateIndices(const GLMeshIndexBufferConfig& indices, std::size_t vertexCount) {
    switch (indices.GetType()) {
        case GLMeshIndexBufferConfig::Type::UInt8:
            ValidateTypedIndices<std::uint8_t>(indices, vertexCount);
            return;
        case GLMeshIndexBufferConfig::Type::UInt16:
            ValidateTypedIndices<std::uint16_t>(indices, vertexCount);
            return;
        case GLMeshIndexBufferConfig::Type::UInt32:
            ValidateTypedIndices<std::uint32_t>(indices, vertexCount);
            return;
    }
    FE_THROW("Unsupported OpenGL mesh index type");
}

std::vector<GLBuffer> UploadAndBindVertexBuffers(
    std::span<const GLMeshVertexBufferConfig> vertexBuffers, GLVertexArray& vertexArray) {
    std::vector<GLBuffer> buffers;
    buffers.reserve(vertexBuffers.size());
    std::size_t binding = 0;
    for (const auto& config : vertexBuffers) {
        GLBuffer buffer;
        buffer.Upload(config.Data(), config.GetUsage());
        vertexArray.SetVertexBuffer(buffer, config.GetLayout(),
                                    static_cast<unsigned int>(binding++));
        buffers.push_back(std::move(buffer));
    }
    return buffers;
}

}  // namespace

GLMesh::IndexFormat GLMesh::GetIndexFormat(GLMeshIndexBufferConfig::Type type) {
    switch (type) {
        case GLMeshIndexBufferConfig::Type::UInt8:
            return {.glType = GL_UNSIGNED_BYTE, .size = sizeof(std::uint8_t)};
        case GLMeshIndexBufferConfig::Type::UInt16:
            return {.glType = GL_UNSIGNED_SHORT, .size = sizeof(std::uint16_t)};
        case GLMeshIndexBufferConfig::Type::UInt32:
            return {.glType = GL_UNSIGNED_INT, .size = sizeof(std::uint32_t)};
    }
    FE_THROW("Unsupported OpenGL mesh index type");
}

GLMesh::GLMesh(std::span<const GLMeshVertexBufferConfig> vertexBuffers,
               const GLMeshIndexBufferConfig& indices) {
    ValidateMeshSizes(vertexBuffers, indices);
    const auto vertexCount = ValidateAndGetVertexCount(vertexBuffers);

    FE_THROW_IF(!indices.HasData(), "OpenGL mesh index buffers require data");
    FE_THROW_IF(indices.Count() == 0, "OpenGL mesh index buffers cannot be empty");
    ValidateIndices(indices, vertexCount);

    indexCount_ = indices.Count();
    indexFormat_ = GetIndexFormat(indices.GetType());

    vertexBuffers_ = UploadAndBindVertexBuffers(vertexBuffers, vertexArray_);
    indexBuffer_.Upload(indices.Data(), indices.GetUsage());
    vertexArray_.SetIndexBuffer(indexBuffer_);
}

GLMesh::GLMesh(GLMesh&& other) noexcept
    : vertexBuffers_(std::move(other.vertexBuffers_)),
      indexBuffer_(std::move(other.indexBuffer_)),
      vertexArray_(std::move(other.vertexArray_)),
      indexCount_(std::exchange(other.indexCount_, 0)),
      indexFormat_(std::exchange(other.indexFormat_, IndexFormat{})) {}

GLMesh& GLMesh::operator=(GLMesh&& other) noexcept {
    if (this != &other) {
        vertexArray_ = std::move(other.vertexArray_);
        indexBuffer_ = std::move(other.indexBuffer_);
        vertexBuffers_ = std::move(other.vertexBuffers_);
        indexCount_ = std::exchange(other.indexCount_, 0);
        indexFormat_ = std::exchange(other.indexFormat_, IndexFormat{});
    }
    return *this;
}

void GLMesh::Draw() const { Draw(0, indexCount_); }

void GLMesh::Draw(std::size_t firstIndex, std::size_t count) const {
    detail::RequireValid(vertexArray_.IsValid(), "draw", "OpenGL mesh");
    FE_THROW_IF(firstIndex > indexCount_ || count > indexCount_ - firstIndex,
                "OpenGL mesh draw range exceeds its active index count");
    FE_THROW_IF(count > static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()),
                "OpenGL mesh draw count is too large");
    vertexArray_.Bind();
    const auto byteOffset = GetBufferSize(firstIndex, indexFormat_.size);
    FE_GL(glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(count), indexFormat_.glType,
                         reinterpret_cast<const void*>(static_cast<std::uintptr_t>(byteOffset))));
}

}  // namespace ferrum::renderer::opengl

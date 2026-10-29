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
    GLMesh(std::span<const GLMeshVertexBufferConfig> vertexBuffers,
           const GLMeshIndexBufferConfig& indices);
    ~GLMesh() = default;
    GLMesh(const GLMesh&) = delete;
    GLMesh& operator=(const GLMesh&) = delete;
    GLMesh(GLMesh&& other) noexcept;
    GLMesh& operator=(GLMesh&& other) noexcept;

    void Draw() const;
    void Draw(std::size_t firstIndex, std::size_t count) const;

private:
    struct IndexFormat {
        unsigned int glType = 0;
        std::size_t size = 0;
    };

    static IndexFormat GetIndexFormat(GLMeshIndexBufferConfig::Type type);

    std::vector<GLBuffer> vertexBuffers_;
    GLBuffer indexBuffer_;
    GLVertexArray vertexArray_;
    std::size_t indexCount_ = 0;
    IndexFormat indexFormat_;
};

}  // namespace ferrum::renderer::opengl

#pragma once

#include "ferrum/renderer/backends/opengl/GLBuffer.hpp"
#include "ferrum/renderer/backends/opengl/LayoutDescriptor.hpp"

namespace ferrum::renderer::opengl {

// Borrows buffers during setup; callers must retain them for drawing.
// GL operations require loaded functions and a compatible current context.
class GLVertexArray {
public:
    GLVertexArray();
    ~GLVertexArray();
    GLVertexArray(const GLVertexArray&) = delete;
    GLVertexArray& operator=(const GLVertexArray&) = delete;
    GLVertexArray(GLVertexArray&& other) noexcept;
    GLVertexArray& operator=(GLVertexArray&& other) noexcept;

    bool IsValid() const noexcept { return id_ != 0; }
    void SetVertexBuffer(const GLBuffer& buffer, const LayoutDescriptor& layout,
                         unsigned int bindingIndex = 0);
    void SetIndexBuffer(const GLBuffer& buffer);
    void Bind() const;
    unsigned int Id() const noexcept { return id_; }

private:
    explicit GLVertexArray(std::nullptr_t) {}

    unsigned int id_ = 0;
};

}  // namespace ferrum::renderer::opengl

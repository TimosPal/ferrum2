#pragma once

#include <cstddef>
#include <span>

namespace ferrum::renderer::opengl {

// GL operations require loaded functions and a compatible current context.
class GLBuffer {
public:
    enum class Usage { StaticDraw, DynamicDraw, StreamDraw };

    GLBuffer();
    ~GLBuffer();
    GLBuffer(const GLBuffer&) = delete;
    GLBuffer& operator=(const GLBuffer&) = delete;
    GLBuffer(GLBuffer&& other) noexcept;
    GLBuffer& operator=(GLBuffer&& other) noexcept;

    bool IsValid() const noexcept { return id_ != 0; }
    void Upload(std::span<const std::byte> data, Usage usage = Usage::StaticDraw);
    void Allocate(std::size_t size, Usage usage = Usage::StaticDraw);
    void Update(std::size_t offset, std::span<const std::byte> data);
    unsigned int Id() const noexcept { return id_; }

private:
    explicit GLBuffer(std::nullptr_t) {}

    unsigned int id_ = 0;
    std::size_t size_ = 0;
};

}  // namespace ferrum::renderer::opengl

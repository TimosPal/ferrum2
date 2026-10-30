#pragma once

#include <cstddef>
#include <initializer_list>
#include <span>
#include <vector>

namespace ferrum::renderer::opengl {

class LayoutDescriptor {
public:
    enum class AttributeType {
        Float,
        Vec2,
        Vec3,
        Vec4,
        Int,
        IVec2,
        IVec3,
        IVec4,
        UInt,
        UVec2,
        UVec3,
        UVec4,
    };

    struct Attribute {
        unsigned int location;
        AttributeType type;
    };

    struct AttributeMetadata {
        Attribute attribute;
        int components;
        std::size_t size;
        std::size_t offset;
        bool is_integer;
    };

    LayoutDescriptor(std::initializer_list<Attribute> attributes);

    std::span<const AttributeMetadata> Metadata() const noexcept { return metadata_; }
    std::size_t Stride() const noexcept { return stride_; }
    bool IsValid() const noexcept { return !metadata_.empty(); }

private:
    void AddAttribute(const Attribute& attribute);

    std::vector<AttributeMetadata> metadata_;
    std::size_t stride_ = 0;
};

}  // namespace ferrum::renderer::opengl

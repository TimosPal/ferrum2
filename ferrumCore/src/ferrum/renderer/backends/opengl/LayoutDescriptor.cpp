#include "ferrum/renderer/backends/opengl/LayoutDescriptor.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>

#include "ferrum/renderer/backends/opengl/utils.hpp"

namespace ferrum::renderer::opengl {

LayoutDescriptor::LayoutDescriptor(std::initializer_list<Attribute> attributes) {
    FE_THROW_IF(attributes.size() == 0, "Vertex layout requires at least one attribute");
    metadata_.reserve(attributes.size());

    for (const auto& attribute : attributes) {
        AddAttribute(attribute);
    }
}

void LayoutDescriptor::AddAttribute(const Attribute& attribute) {
    const auto duplicate =
        std::ranges::find_if(metadata_, [&attribute](const AttributeMetadata& existing) {
            return existing.attribute.location == attribute.location;
        });
    FE_THROW_IF(duplicate != metadata_.end(), "Duplicate vertex attribute location {}",
                attribute.location);

    AttributeMetadata metadata{
        .attribute = attribute, .components = 0, .size = 0, .offset = stride_, .is_integer = false};
    switch (attribute.type) {
        case AttributeType::Float:
            metadata.components = 1;
            metadata.size = sizeof(float);
            break;
        case AttributeType::Vec2:
            metadata.components = 2;
            metadata.size = 2 * sizeof(float);
            break;
        case AttributeType::Vec3:
            metadata.components = 3;
            metadata.size = 3 * sizeof(float);
            break;
        case AttributeType::Vec4:
            metadata.components = 4;
            metadata.size = 4 * sizeof(float);
            break;
        case AttributeType::Int:
            metadata.components = 1;
            metadata.size = sizeof(std::int32_t);
            metadata.is_integer = true;
            break;
        case AttributeType::IVec2:
            metadata.components = 2;
            metadata.size = 2 * sizeof(std::int32_t);
            metadata.is_integer = true;
            break;
        case AttributeType::IVec3:
            metadata.components = 3;
            metadata.size = 3 * sizeof(std::int32_t);
            metadata.is_integer = true;
            break;
        case AttributeType::IVec4:
            metadata.components = 4;
            metadata.size = 4 * sizeof(std::int32_t);
            metadata.is_integer = true;
            break;
        case AttributeType::UInt:
            metadata.components = 1;
            metadata.size = sizeof(std::uint32_t);
            metadata.is_integer = true;
            break;
        case AttributeType::UVec2:
            metadata.components = 2;
            metadata.size = 2 * sizeof(std::uint32_t);
            metadata.is_integer = true;
            break;
        case AttributeType::UVec3:
            metadata.components = 3;
            metadata.size = 3 * sizeof(std::uint32_t);
            metadata.is_integer = true;
            break;
        case AttributeType::UVec4:
            metadata.components = 4;
            metadata.size = 4 * sizeof(std::uint32_t);
            metadata.is_integer = true;
            break;
        default:
            FE_THROW("Unsupported vertex layout attribute type");
    }

    FE_THROW_IF(metadata.size > std::numeric_limits<std::size_t>::max() - stride_,
                "Vertex layout stride is too large");
    metadata_.push_back(metadata);
    stride_ += metadata.size;
}

}  // namespace ferrum::renderer::opengl

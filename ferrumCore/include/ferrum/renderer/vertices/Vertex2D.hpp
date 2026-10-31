#pragma once

#include <glm/vec2.hpp>

#include "ferrum/graphics/Color.hpp"
#include "ferrum/renderer/LayoutDescriptor.hpp"

namespace ferrum::renderer::vertices {

struct Vertex2D {
    float x = 0.0F;
    float y = 0.0F;
    glm::vec2 uv{0.0F};
    graphics::Color tint = graphics::White;

    static LayoutDescriptor Layout() {
        return {
            // Position
            {.location = 0, .type = LayoutDescriptor::AttributeType::Vec2},
            // UV
            {.location = 1, .type = LayoutDescriptor::AttributeType::Vec2},
            // Tint
            {.location = 2, .type = LayoutDescriptor::AttributeType::Vec4},
        };
    }
};

}  // namespace ferrum::renderer::vertices

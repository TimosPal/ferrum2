#pragma once

#include "ferrum/renderer/LayoutDescriptor.hpp"

namespace ferrum::renderer::vertices {

struct Vertex2D {
    float x = 0.0F;
    float y = 0.0F;

    static LayoutDescriptor Layout() {
        return {{.location = 0, .type = LayoutDescriptor::AttributeType::Vec2}};
    }
};

}  // namespace ferrum::renderer::vertices

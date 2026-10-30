#pragma once

#include <array>
#include <cstdint>

#include "ferrum/renderer/vertices/Vertex2D.hpp"

namespace ferrum::renderer::primitives {

struct Quad {
    inline static constexpr std::array<vertices::Vertex2D, 4> vertices{{
        {.x = -0.5F, .y = -0.5F},
        {.x = 0.5F, .y = -0.5F},
        {.x = 0.5F, .y = 0.5F},
        {.x = -0.5F, .y = 0.5F},
    }};
    inline static constexpr std::array<std::uint16_t, 6> indices{0, 1, 2, 2, 3, 0};
};

}  // namespace ferrum::renderer::primitives

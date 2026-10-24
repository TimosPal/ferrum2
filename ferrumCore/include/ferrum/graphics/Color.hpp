#pragma once

#include <glm/glm.hpp>

namespace ferrum::graphics {

struct Color {
    float r = 0.0F;
    float g = 0.0F;
    float b = 0.0F;
    float a = 1.0F;

    glm::vec4 ToVec4() const noexcept { return {r, g, b, a}; }
};

inline constexpr Color Black{.r = 0.0F, .g = 0.0F, .b = 0.0F};
inline constexpr Color White{.r = 1.0F, .g = 1.0F, .b = 1.0F};
inline constexpr Color Red{.r = 1.0F, .g = 0.0F, .b = 0.0F};
inline constexpr Color Green{.r = 0.0F, .g = 1.0F, .b = 0.0F};
inline constexpr Color Blue{.r = 0.0F, .g = 0.0F, .b = 1.0F};
inline constexpr Color Transparent{.r = 0.0F, .g = 0.0F, .b = 0.0F, .a = 0.0F};

}  // namespace ferrum::graphics

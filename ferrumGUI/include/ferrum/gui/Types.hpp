#pragma once

#include <algorithm>
#include <glm/vec4.hpp>

namespace ferrum::gui {

// Coordinates use pixels with a top-left origin, X right and Y down.
struct Rect {
    float x = 0.0F;
    float y = 0.0F;
    float width = 0.0F;
    float height = 0.0F;
};

// Straight-alpha RGBA components in the range [0, 1].
struct Color {
    float r = 0.0F;
    float g = 0.0F;
    float b = 0.0F;
    float a = 1.0F;
};

// Corner radii are in framebuffer pixels, ordered clockwise from the top-left.
struct CornerRadii {
    float top_left = 0.0F;
    float top_right = 0.0F;
    float bottom_right = 0.0F;
    float bottom_left = 0.0F;

    glm::vec4 ToVec4() const noexcept { return {top_left, top_right, bottom_right, bottom_left}; }

    // Applies CSS border-radius normalization, uniformly scaling adjacent radii to fit the
    // rectangle. Requires positive dimensions and finite, non-negative radii.
    CornerRadii FitWithin(const Rect& bounds) const noexcept {
        auto scale = 1.0;
        const auto top_sum = static_cast<double>(top_left) + top_right;
        const auto bottom_sum = static_cast<double>(bottom_left) + bottom_right;
        const auto left_sum = static_cast<double>(top_left) + bottom_left;
        const auto right_sum = static_cast<double>(top_right) + bottom_right;
        if (top_sum > bounds.width) {
            scale = std::min(scale, bounds.width / top_sum);
        }
        if (bottom_sum > bounds.width) {
            scale = std::min(scale, bounds.width / bottom_sum);
        }
        if (left_sum > bounds.height) {
            scale = std::min(scale, bounds.height / left_sum);
        }
        if (right_sum > bounds.height) {
            scale = std::min(scale, bounds.height / right_sum);
        }

        return {
            .top_left = static_cast<float>(static_cast<double>(top_left) * scale),
            .top_right = static_cast<float>(static_cast<double>(top_right) * scale),
            .bottom_right = static_cast<float>(static_cast<double>(bottom_right) * scale),
            .bottom_left = static_cast<float>(static_cast<double>(bottom_left) * scale),
        };
    }
};

}  // namespace ferrum::gui

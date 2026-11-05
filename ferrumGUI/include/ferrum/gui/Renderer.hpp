#pragma once

#include "ferrum/gui/Types.hpp"

namespace ferrum::gui {

// The GUI renderer owns its frame lifecycle; do not mix it with the core renderer's frame calls.
class Renderer {
public:
    virtual ~Renderer() = default;
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    // Returns false when the framebuffer has no drawable area.
    virtual bool BeginFrame() = 0;
    virtual void Clear(const Color& color) = 0;

    // Zero-sized rectangles draw nothing; invalid bounds, colors, or radii must report an error.
    // Overlapping radii are uniformly scaled down to fit within the rectangle.
    virtual void FillRect(const Rect& bounds, const Color& color,
                          const CornerRadii& corner_radii = {}) = 0;
    virtual void EndFrame() = 0;

protected:
    Renderer() = default;
};

}  // namespace ferrum::gui

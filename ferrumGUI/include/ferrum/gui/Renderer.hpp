#pragma once

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

    // Zero-sized rectangles draw nothing; invalid bounds or colors must report an error.
    virtual void FillRect(const Rect& bounds, const Color& color) = 0;
    virtual void EndFrame() = 0;

protected:
    Renderer() = default;
};

}  // namespace ferrum::gui

#pragma once

#include "ferrum/graphics/Color.hpp"

namespace ferrum::renderer {

class Renderer {
public:
    virtual ~Renderer() = default;
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    virtual void Clear(const graphics::Color& color) = 0;
    virtual void Present() = 0;

protected:
    Renderer() = default;
};

}  // namespace ferrum::renderer
#pragma once

#include <memory>

#include "ferrum/graphics/Color.hpp"
#include "ferrum/graphics/WindowGraphicsBinding.hpp"

namespace ferrum::renderer {

class Renderer {
public:
    static std::unique_ptr<Renderer> Create(graphics::WindowGraphicsBinding* binding);

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
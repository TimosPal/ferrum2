#pragma once

#include <memory>

#include "ferrum/graphics/GraphicsAPI.hpp"
#include "ferrum/window/Window.hpp"
#include "ferrum/window/WindowDesc.hpp"

namespace ferrum::core {

class Platform {
public:
    static std::unique_ptr<Platform> Create();

    virtual ~Platform() = default;
    Platform(const Platform&) = delete;
    Platform& operator=(const Platform&) = delete;
    Platform(Platform&&) = delete;
    Platform& operator=(Platform&&) = delete;

    virtual std::unique_ptr<ferrum::window::Window> CreateWindow(
        const ferrum::window::WindowDesc& data, ferrum::graphics::GraphicsAPI api) = 0;

protected:
    Platform() = default;
};

}  // namespace ferrum::core
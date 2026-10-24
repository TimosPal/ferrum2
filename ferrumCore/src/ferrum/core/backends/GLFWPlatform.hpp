#pragma once

#include "ferrum/core/Platform.hpp"

namespace ferrum::core::backends {

class GLFWPlatform final : public Platform {
public:
    GLFWPlatform();
    ~GLFWPlatform() override;

    GLFWPlatform(const GLFWPlatform&) = delete;
    GLFWPlatform& operator=(const GLFWPlatform&) = delete;
    GLFWPlatform(GLFWPlatform&&) = delete;
    GLFWPlatform& operator=(GLFWPlatform&&) = delete;

    std::unique_ptr<ferrum::window::Window> CreateWindow(
        const ferrum::window::WindowDesc& data, ferrum::graphics::GraphicsAPI api) override;
};

}  // namespace ferrum::core::backends
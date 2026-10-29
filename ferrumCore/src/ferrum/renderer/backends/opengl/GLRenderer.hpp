#pragma once

#include "ferrum/graphics/OpenGLWindowBinding.hpp"
#include "ferrum/renderer/Renderer.hpp"

namespace ferrum::renderer::opengl {

class GLRenderer : public renderer::Renderer {
public:
    explicit GLRenderer(graphics::OpenGLWindowBinding& binding);
    GLRenderer(const GLRenderer&) = delete;
    GLRenderer& operator=(const GLRenderer&) = delete;
    GLRenderer(GLRenderer&&) = delete;
    GLRenderer& operator=(GLRenderer&&) = delete;
    ~GLRenderer() override = default;

    void Clear(const graphics::Color& color) override;
    void Present() override;

private:
    graphics::OpenGLWindowBinding& binding_;
};

}  // namespace ferrum::renderer::opengl
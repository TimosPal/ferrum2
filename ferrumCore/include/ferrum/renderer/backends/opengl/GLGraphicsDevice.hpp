#pragma once

#include "ferrum/graphics/GraphicsDevice.hpp"
#include "ferrum/graphics/OpenGLWindowBinding.hpp"

namespace ferrum::renderer::opengl {

class GLGraphicsDevice final : public graphics::GraphicsDevice {
public:
    explicit GLGraphicsDevice(graphics::OpenGLWindowBinding& binding) : binding_(binding) {}
    ~GLGraphicsDevice() override = default;

    GLGraphicsDevice(const GLGraphicsDevice&) = delete;
    GLGraphicsDevice& operator=(const GLGraphicsDevice&) = delete;
    GLGraphicsDevice(GLGraphicsDevice&&) = delete;
    GLGraphicsDevice& operator=(GLGraphicsDevice&&) = delete;

    std::unique_ptr<renderer::Renderer> CreateRenderer() override;
    std::unique_ptr<renderer::ShaderModule> CreateShaderModule(
        const renderer::ShaderModuleDesc& description) override;
    std::unique_ptr<renderer::ProgramModule> CreateProgramModule(
        const renderer::ProgramModuleDesc& description) override;
    std::unique_ptr<renderer::Mesh> CreateMesh(const renderer::MeshDesc& description) override;
    std::unique_ptr<renderer::Texture> CreateTexture(
        const renderer::TextureDesc& description, renderer::TextureOptions options = {}) override;

private:
    graphics::OpenGLWindowBinding& binding_;
};

}  // namespace ferrum::renderer::opengl

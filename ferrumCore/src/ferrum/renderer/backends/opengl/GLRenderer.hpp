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

    void BeginFrame(const renderer::CameraMatrices& camera_matrices) override;
    void Clear(const graphics::Color& color) override;
    void Draw(
        renderer::Material& material, renderer::Mesh& mesh,
        renderer::PrimitiveTopology topology = renderer::PrimitiveTopology::Triangles) override;
    void Draw(
        renderer::Material& material, renderer::Mesh& mesh, std::size_t index_count,
        std::size_t index_offset,
        renderer::PrimitiveTopology topology = renderer::PrimitiveTopology::Triangles) override;
    void EndFrame() override;

private:
    void DrawProgram(const renderer::ProgramModule& program, renderer::Mesh& mesh,
                     renderer::PrimitiveTopology topology);
    void DrawProgram(const renderer::ProgramModule& program, renderer::Mesh& mesh,
                     std::size_t index_count, std::size_t index_offset,
                     renderer::PrimitiveTopology topology);

    graphics::OpenGLWindowBinding& binding_;
    const renderer::CameraMatrices* camera_matrices_ = nullptr;
};

}  // namespace ferrum::renderer::opengl
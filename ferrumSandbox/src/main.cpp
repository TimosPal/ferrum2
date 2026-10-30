#include <cstdint>
#include <filesystem>
#include <span>

#include "ferrum/core/Platform.hpp"
#include "ferrum/renderer/Renderer.hpp"
#include "ferrum/renderer/backends/opengl/GLMesh.hpp"
#include "ferrum/renderer/backends/opengl/GLProgram.hpp"
#include "ferrum/renderer/backends/opengl/GLShader.hpp"
#include "ferrum/renderer/primitives/Quad.hpp"
#include "ferrum/resources/ResourceManager.hpp"

int main() {
    using ferrum::core::Platform;
    using ferrum::graphics::Color;
    using ferrum::graphics::GraphicsAPI;
    using ferrum::renderer::Renderer;
    using ferrum::renderer::opengl::GLMesh;
    using ferrum::renderer::opengl::GLMeshIndexBufferConfig;
    using ferrum::renderer::opengl::GLMeshVertexBufferConfig;
    using ferrum::renderer::opengl::GLProgram;
    using ferrum::renderer::opengl::GLShader;
    using ferrum::renderer::primitives::Quad;
    using ferrum::renderer::vertices::Vertex2D;
    using ferrum::resources::ResourceManager;
    using ferrum::resources::types::TextResource;

    constexpr auto graphics_api = GraphicsAPI::OpenGL;
    auto platform = Platform::Create();
    auto window = platform->CreateWindow({.title = "Ferrum Window", .width = 800, .height = 600},
                                         graphics_api);
    auto renderer = Renderer::Create(window->GetGraphicsBinding());

    ResourceManager resources;
    const auto shader_directory =
        std::filesystem::path(FERRUM_CORE_ASSET_ROOT) / "shaders" / "opengl";
    auto vertex_shader_source =
        resources.LoadResource<TextResource>(shader_directory / "quad.vert");
    auto fragment_shader_source =
        resources.LoadResource<TextResource>(shader_directory / "quad.frag");
    GLShader vertex_shader(GLShader::Type::Vertex, vertex_shader_source.handle);
    GLShader fragment_shader(GLShader::Type::Fragment, fragment_shader_source.handle);
    GLProgram program(vertex_shader, fragment_shader);

    const GLMeshVertexBufferConfig vertex_buffer{
        std::span<const Vertex2D>(Quad::vertices), Vertex2D::Layout(),
    };
    const GLMeshIndexBufferConfig index_buffer{
        std::span<const std::uint16_t>(Quad::indices),
    };
    GLMesh mesh(vertex_buffer, index_buffer);

    const Color background_color{.r = 0.12F, .g = 0.2F, .b = 0.32F};
    const auto render_frame = [&] {
        renderer->Clear(background_color);
        program.Use();
        mesh.Draw();
        renderer->Present();
    };
    window->SetRefreshCallback(render_frame);

    while (!window->ShouldClose()) {
        window->PollEvents();
        render_frame();
    }
    return 0;
}

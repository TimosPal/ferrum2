#include <cstdint>
#include <filesystem>
#include <span>

#include "ferrum/core/Platform.hpp"
#include "ferrum/graphics/Color.hpp"
#include "ferrum/graphics/GraphicsDevice.hpp"
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
    using ferrum::graphics::GraphicsDevice;
    using ferrum::renderer::ProgramModuleDesc;
    using ferrum::renderer::ShaderType;
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
    auto device = GraphicsDevice::Create(window->GetGraphicsBinding());
    auto renderer = device->CreateRenderer();

    ResourceManager resources;
    const auto shader_directory =
        std::filesystem::path(FERRUM_CORE_ASSET_ROOT) / "shaders" / "opengl";
    auto vertex_shader_source =
        resources.LoadResource<TextResource>(shader_directory / "quad.vert");
    auto fragment_shader_source =
        resources.LoadResource<TextResource>(shader_directory / "quad.frag");
    GLShader vertex_shader(ShaderType::Vertex,
                           vertex_shader_source.handle.GetResource()->GetText());
    GLShader fragment_shader(ShaderType::Fragment,
                             fragment_shader_source.handle.GetResource()->GetText());
    const ProgramModuleDesc program_description{
        .vertex_shader = vertex_shader,
        .fragment_shader = fragment_shader,
    };
    GLProgram program(program_description);

    const GLMeshVertexBufferConfig vertex_buffer{
        std::span<const Vertex2D>(Quad::vertices),
        Vertex2D::Layout(),
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

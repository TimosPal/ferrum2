#include <array>
#include <cstdint>
#include <filesystem>
#include <span>

#include "ferrum/core/Platform.hpp"
#include "ferrum/renderer/Renderer.hpp"
#include "ferrum/renderer/backends/opengl/GLMesh.hpp"
#include "ferrum/renderer/backends/opengl/GLProgram.hpp"
#include "ferrum/renderer/backends/opengl/GLShader.hpp"
#include "ferrum/resources/ResourceManager.hpp"

int main() {
    constexpr auto graphics_api = ferrum::graphics::GraphicsAPI::OpenGL;
    auto platform = ferrum::core::Platform::Create();
    auto window = platform->CreateWindow({.title = "Ferrum Window", .width = 800, .height = 600},
                                         graphics_api);
    auto renderer = ferrum::renderer::Renderer::Create(window->GetGraphicsBinding());

    ferrum::resources::ResourceManager resources;
    const auto shader_directory =
        std::filesystem::path(FERRUM_CORE_ASSET_ROOT) / "shaders" / "opengl";
    auto vertex_shader_source = resources.LoadResource<ferrum::resources::types::TextResource>(
        shader_directory / "quad.vert");
    auto fragment_shader_source = resources.LoadResource<ferrum::resources::types::TextResource>(
        shader_directory / "quad.frag");
    ferrum::renderer::opengl::GLShader vertex_shader(
        ferrum::renderer::opengl::GLShader::Type::Vertex, vertex_shader_source.handle);
    ferrum::renderer::opengl::GLShader fragment_shader(
        ferrum::renderer::opengl::GLShader::Type::Fragment, fragment_shader_source.handle);
    ferrum::renderer::opengl::GLProgram program(vertex_shader, fragment_shader);

    struct Vertex {
        float x;
        float y;
    };
    const std::array vertices{
        Vertex{.x = -.5F, .y = -.5F},
        Vertex{.x = .5F, .y = -.5F},
        Vertex{.x = .5F, .y = .5F},
        Vertex{.x = -.5F, .y = .5F},
    };
    const std::array<std::uint16_t, 6> indices{0, 1, 2, 2, 3, 0};
    const ferrum::renderer::opengl::LayoutDescriptor vertex_layout{
        {.location = 0, .type = ferrum::renderer::opengl::LayoutDescriptor::AttributeType::Vec2},
    };
    const std::array vertex_buffers{
        ferrum::renderer::opengl::GLMeshVertexBufferConfig(std::span<const Vertex>(vertices),
                                                           vertex_layout),
    };
    const ferrum::renderer::opengl::GLMeshIndexBufferConfig index_buffer{
        std::span<const std::uint16_t>(indices),
    };
    ferrum::renderer::opengl::GLMesh mesh(vertex_buffers, index_buffer);

    const ferrum::graphics::Color background_color{.r = 0.12F, .g = 0.2F, .b = 0.32F};
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

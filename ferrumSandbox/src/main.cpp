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
    constexpr auto graphicsAPI = ferrum::graphics::GraphicsAPI::OpenGL;
    auto platform = ferrum::core::Platform::Create();
    auto window = platform->CreateWindow({.title = "Ferrum Window", .width = 800, .height = 600},
                                         graphicsAPI);
    auto renderer = ferrum::renderer::Renderer::Create(window->GetGraphicsBinding());

    ferrum::resources::ResourceManager resources;
    const auto shaderDirectory =
        std::filesystem::path(FERRUM_CORE_ASSET_ROOT) / "shaders" / "opengl";
    auto vertexShaderSource = resources.LoadResource<ferrum::resources::types::TextResource>(
        shaderDirectory / "quad.vert");
    auto fragmentShaderSource = resources.LoadResource<ferrum::resources::types::TextResource>(
        shaderDirectory / "quad.frag");
    ferrum::renderer::opengl::GLShader vertexShader(
        ferrum::renderer::opengl::GLShader::Type::Vertex, vertexShaderSource.handle);
    ferrum::renderer::opengl::GLShader fragmentShader(
        ferrum::renderer::opengl::GLShader::Type::Fragment, fragmentShaderSource.handle);
    ferrum::renderer::opengl::GLProgram program(vertexShader, fragmentShader);

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
    const ferrum::renderer::opengl::LayoutDescriptor vertexLayout{
        {.location = 0, .type = ferrum::renderer::opengl::LayoutDescriptor::AttributeType::Vec2},
    };
    const std::array vertexBuffers{
        ferrum::renderer::opengl::GLMeshVertexBufferConfig(std::span<const Vertex>(vertices),
                                                           vertexLayout),
    };
    const ferrum::renderer::opengl::GLMeshIndexBufferConfig indexBuffer{
        std::span<const std::uint16_t>(indices),
    };
    ferrum::renderer::opengl::GLMesh mesh(vertexBuffers, indexBuffer);

    const ferrum::graphics::Color backgroundColor{.r = 0.12F, .g = 0.2F, .b = 0.32F};
    const auto renderFrame = [&] {
        renderer->Clear(backgroundColor);
        program.Use();
        mesh.Draw();
        renderer->Present();
    };
    window->SetRefreshCallback(renderFrame);

    while (!window->ShouldClose()) {
        window->PollEvents();
        renderFrame();
    }
    return 0;
}

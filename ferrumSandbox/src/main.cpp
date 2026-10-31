#include <cstdint>
#include <filesystem>
#include <span>

#include "ferrum/core/Platform.hpp"
#include "ferrum/graphics/Color.hpp"
#include "ferrum/graphics/GraphicsDevice.hpp"
#include "ferrum/renderer/Camera2D.hpp"
#include "ferrum/renderer/Material.hpp"
#include "ferrum/renderer/Mesh.hpp"
#include "ferrum/renderer/MeshConfig.hpp"
#include "ferrum/renderer/ProgramModule.hpp"
#include "ferrum/renderer/Renderer.hpp"
#include "ferrum/renderer/ShaderModule.hpp"
#include "ferrum/renderer/primitives/Quad.hpp"
#include "ferrum/resources/ResourceManager.hpp"
#include "ferrum/resources/types/ImageResource.hpp"

int main() {
    using ferrum::core::Platform;
    using ferrum::graphics::Color;
    using ferrum::graphics::GraphicsAPI;
    using ferrum::graphics::GraphicsDevice;
    using ferrum::renderer::ShaderType;
    using ferrum::renderer::primitives::Quad;
    using ferrum::renderer::vertices::Vertex2D;
    using ferrum::resources::ResourceManager;
    using ferrum::resources::types::ImageResource;
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
    const ferrum::renderer::ShaderModuleDesc vertex_shader_description{
        .type = ShaderType::Vertex,
        .source = vertex_shader_source.handle.GetResource()->GetText(),
    };
    const ferrum::renderer::ShaderModuleDesc fragment_shader_description{
        .type = ShaderType::Fragment,
        .source = fragment_shader_source.handle.GetResource()->GetText(),
    };
    auto vertex_shader = device->CreateShaderModule(vertex_shader_description);
    auto fragment_shader = device->CreateShaderModule(fragment_shader_description);
    const ferrum::renderer::ProgramModuleDesc program_description{
        .vertex_shader = *vertex_shader,
        .fragment_shader = *fragment_shader,
    };
    auto program_module = device->CreateProgramModule(program_description);

    const ferrum::renderer::VertexBufferConfig vertex_buffer{
        std::span<const Vertex2D>(Quad::vertices),
        Vertex2D::Layout(),
    };
    const ferrum::renderer::IndexBufferConfig index_buffer{
        std::span<const std::uint16_t>(Quad::indices),
    };
    const ferrum::renderer::MeshDesc mesh_description{vertex_buffer, index_buffer};
    auto mesh_resource = device->CreateMesh(mesh_description);

    ImageResource image(1, 1, 4, {255, 255, 255, 255});
    auto texture = device->CreateTexture(
        {.width = image.GetWidth(), .height = image.GetHeight(), .channels = image.GetChannels()});
    texture->Upload(image);

    ferrum::renderer::Material material(*program_module);
    material.SetTexture("spriteTexture", *texture);
    ferrum::renderer::Camera2D camera;
    const Color background_color{.r = 0.12F, .g = 0.2F, .b = 0.32F};
    const auto render_frame = [&] {
        const auto framebuffer_size = window->GetFramebufferSize();
        camera.SetAspectRatio(static_cast<float>(framebuffer_size.x) /
                              static_cast<float>(framebuffer_size.y));
        renderer->BeginFrame(camera.GetMatrices());
        renderer->Clear(background_color);
        renderer->Draw(material, *mesh_resource);
        renderer->EndFrame();
    };
    window->SetRefreshCallback(render_frame);

    while (!window->ShouldClose()) {
        window->PollEvents();
        render_frame();
    }
    return 0;
}

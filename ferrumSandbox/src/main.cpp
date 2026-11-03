#include <cstdint>
#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>
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
#include "ferrum/renderer/Texture.hpp"
#include "ferrum/renderer/primitives/Quad.hpp"
#include "ferrum/resources/ResourceManager.hpp"
#include "ferrum/resources/types/ImageResource.hpp"

int main() {
    using ferrum::core::Platform;
    using ferrum::graphics::Color;
    using ferrum::graphics::GraphicsAPI;
    using ferrum::graphics::GraphicsDevice;
    using ferrum::renderer::ProgramModule;
    using ferrum::renderer::ShaderModule;
    using ferrum::renderer::ShaderType;
    using ferrum::renderer::Texture;
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
    auto vertex_shader = resources.CreateDeferredResource<ShaderModule>({
        .device = *device,
        .type = ShaderType::Vertex,
        .source = vertex_shader_source.handle,
    });
    auto fragment_shader = resources.CreateDeferredResource<ShaderModule>({
        .device = *device,
        .type = ShaderType::Fragment,
        .source = fragment_shader_source.handle,
    });
    auto program = resources.CreateDeferredResource<ProgramModule>({
        .device = *device,
        .vertex_shader = vertex_shader.handle,
        .fragment_shader = fragment_shader.handle,
    });

    const ferrum::renderer::VertexBufferConfig vertex_buffer{
        std::span<const Vertex2D>(Quad::vertices),
        Vertex2D::Layout(),
    };
    const ferrum::renderer::IndexBufferConfig index_buffer{
        std::span<const std::uint16_t>(Quad::indices),
    };
    const ferrum::renderer::MeshDesc mesh_description{vertex_buffer, index_buffer};
    auto mesh_resource = device->CreateMesh(mesh_description);

    auto image = resources.CreateResource(ImageResource(1, 1, 4, {255, 255, 255, 255}));
    auto texture = resources.CreateDeferredResource<Texture>({
        .device = *device,
        .image = image.handle,
    });

    ferrum::renderer::Material material(program.handle, {{"spriteTexture", texture.handle}});
    auto left_model = glm::translate(glm::mat4(1.0F), glm::vec3(-0.92F, -0.25F, 0.0F));
    left_model = glm::rotate(left_model, 0.35F, glm::vec3(0.0F, 0.0F, 1.0F));
    left_model = glm::scale(left_model, glm::vec3(0.4F, 0.4F, 1.0F));
    auto right_model = glm::translate(glm::mat4(1.0F), glm::vec3(0.92F, 0.25F, 0.0F));
    right_model = glm::rotate(right_model, -0.4F, glm::vec3(0.0F, 0.0F, 1.0F));
    right_model = glm::scale(right_model, glm::vec3(0.45F, 0.65F, 1.0F));
    ferrum::renderer::Camera2D camera;
    const Color background_color{.r = 0.12F, .g = 0.2F, .b = 0.32F};
    const auto render_frame = [&] {
        const auto framebuffer_size = window->GetFramebufferSize();
        camera.SetAspectRatio(static_cast<float>(framebuffer_size.x) /
                              static_cast<float>(framebuffer_size.y));
        renderer->BeginFrame(camera.GetMatrices());
        renderer->Clear(background_color);
        renderer->Draw(material, *mesh_resource, left_model);
        renderer->Draw(material, *mesh_resource);
        renderer->Draw(material, *mesh_resource, right_model);
        renderer->EndFrame();
    };
    window->SetRefreshCallback(render_frame);

    while (!window->ShouldClose()) {
        window->PollEvents();
        render_frame();
    }
    return 0;
}

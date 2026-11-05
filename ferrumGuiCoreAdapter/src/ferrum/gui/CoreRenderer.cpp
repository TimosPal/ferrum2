#include "ferrum/gui/CoreRenderer.hpp"

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>
#include <span>
#include <utility>

#include "ferrum/common/exception.hpp"
#include "ferrum/graphics/GraphicsDevice.hpp"
#include "ferrum/renderer/Renderer.hpp"
#include "ferrum/renderer/primitives/Quad.hpp"
#include "ferrum/resources/ResourceManager.hpp"

namespace ferrum::gui {
namespace {

auto RequireFramebufferSizeProvider(std::function<glm::ivec2()> provider) {
    FE_THROW_IF(!provider, "GUI framebuffer size provider cannot be empty");
    return provider;
}

auto CreateProgram(graphics::GraphicsDevice& device, resources::ResourceManager& resources) {
    const auto shader_directory =
        std::filesystem::path(FERRUM_GUI_CORE_ADAPTER_ASSET_ROOT) / "shaders" / "opengl";
    auto vertex_source =
        resources.LoadResource<resources::types::TextResource>(shader_directory / "fill_rect.vert");
    auto fragment_source =
        resources.LoadResource<resources::types::TextResource>(shader_directory / "fill_rect.frag");
    auto vertex = resources.CreateDeferredResource<renderer::ShaderModule>({
        .device = device,
        .type = renderer::ShaderType::Vertex,
        .source = vertex_source.handle,
    });
    auto fragment = resources.CreateDeferredResource<renderer::ShaderModule>({
        .device = device,
        .type = renderer::ShaderType::Fragment,
        .source = fragment_source.handle,
    });
    return resources
        .CreateDeferredResource<renderer::ProgramModule>({
            .device = device,
            .vertex_shader = vertex.handle,
            .fragment_shader = fragment.handle,
        })
        .handle;
}

auto CreateQuad(graphics::GraphicsDevice& device) {
    const renderer::VertexBufferConfig vertices{
        std::span<const renderer::vertices::Vertex2D>(renderer::primitives::Quad::vertices),
        renderer::vertices::Vertex2D::Layout(),
    };
    const renderer::IndexBufferConfig indices{
        std::span<const std::uint16_t>(renderer::primitives::Quad::indices),
    };
    auto mesh = device.CreateMesh({vertices, indices});
    FE_THROW_IF(mesh == nullptr, "GUI quad creation returned null");
    return mesh;
}

}  // namespace

CoreRenderer::CoreRenderer(renderer::Renderer& renderer, graphics::GraphicsDevice& device,
                           resources::ResourceManager& resources,
                           std::function<glm::ivec2()> framebuffer_size_provider)
    : renderer_(renderer),
      framebuffer_size_provider_(
          RequireFramebufferSizeProvider(std::move(framebuffer_size_provider))),
      material_(CreateProgram(device, resources), {}),
      quad_(CreateQuad(device)) {}

bool CoreRenderer::BeginFrame() {
    const auto framebuffer_size = framebuffer_size_provider_();
    if (framebuffer_size.x <= 0 || framebuffer_size.y <= 0) {
        return false;
    }
    camera_matrices_.projection =
        glm::ortho(0.0F, static_cast<float>(framebuffer_size.x),
                   static_cast<float>(framebuffer_size.y), 0.0F, -1.0F, 1.0F);
    camera_matrices_.view_projection = camera_matrices_.projection * camera_matrices_.view;
    renderer_.BeginFrame(camera_matrices_);
    return true;
}

void CoreRenderer::Clear(const Color& color) {
    renderer_.Clear({.r = color.r, .g = color.g, .b = color.b, .a = color.a});
}

void CoreRenderer::FillRect(const Rect& bounds, const Color& color,
                            const CornerRadii& corner_radii) {
    FE_THROW_IF(!std::isfinite(bounds.x) || !std::isfinite(bounds.y) ||
                    !std::isfinite(bounds.width) || !std::isfinite(bounds.height),
                "GUI rectangle bounds must be finite");
    FE_THROW_IF(bounds.width < 0.0F || bounds.height < 0.0F,
                "GUI rectangle dimensions cannot be negative");
    FE_THROW_IF(!std::isfinite(bounds.x + bounds.width) || !std::isfinite(bounds.y + bounds.height),
                "GUI rectangle bounds exceed the supported range");
    const auto valid_component = [](float value) {
        return std::isfinite(value) && value >= 0.0F && value <= 1.0F;
    };
    FE_THROW_IF(!valid_component(color.r) || !valid_component(color.g) ||
                    !valid_component(color.b) || !valid_component(color.a),
                "GUI color components must be finite and in [0, 1]");
    const auto valid_radius = [](float radius) { return std::isfinite(radius) && radius >= 0.0F; };
    FE_THROW_IF(!valid_radius(corner_radii.top_left) || !valid_radius(corner_radii.top_right) ||
                    !valid_radius(corner_radii.bottom_right) ||
                    !valid_radius(corner_radii.bottom_left),
                "GUI corner radii must be finite and non-negative");
    if (bounds.width == 0.0F || bounds.height == 0.0F) {
        return;
    }

    auto model = glm::translate(
        glm::mat4(1.0F),
        glm::vec3(bounds.x + (bounds.width * 0.5F), bounds.y + (bounds.height * 0.5F), 0.0F));
    model = glm::scale(model, glm::vec3(bounds.width, bounds.height, 1.0F));
    material_.SetParameter("color", glm::vec4(color.r, color.g, color.b, color.a));
    material_.SetParameter("rectSize", glm::vec2(bounds.width, bounds.height));
    material_.SetParameter("cornerRadii", corner_radii.FitWithin(bounds).ToVec4());
    renderer_.Draw(material_, *quad_, model);
}

void CoreRenderer::EndFrame() { renderer_.EndFrame(); }

}  // namespace ferrum::gui

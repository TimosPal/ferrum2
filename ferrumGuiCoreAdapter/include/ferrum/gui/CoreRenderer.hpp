#pragma once

#include <functional>
#include <glm/vec2.hpp>
#include <memory>

#include "ferrum/gui/Renderer.hpp"
#include "ferrum/renderer/CameraMatrices.hpp"
#include "ferrum/renderer/Material.hpp"
#include "ferrum/renderer/Mesh.hpp"

namespace ferrum::graphics {
class GraphicsDevice;
}

namespace ferrum::renderer {
class Renderer;
}

namespace ferrum::resources {
class ResourceManager;
}

namespace ferrum::gui {

// Renderer, device, and resource manager must outlive the adapter.
class CoreRenderer final : public Renderer {
public:
    CoreRenderer(renderer::Renderer& renderer, graphics::GraphicsDevice& device,
                 resources::ResourceManager& resources,
                 std::function<glm::ivec2()> framebuffer_size_provider);

    bool BeginFrame() override;
    void Clear(const Color& color) override;
    void FillRect(const Rect& bounds, const Color& color) override;
    void EndFrame() override;

private:
    renderer::Renderer& renderer_;
    std::function<glm::ivec2()> framebuffer_size_provider_;
    renderer::CameraMatrices camera_matrices_;
    renderer::Material material_;
    std::unique_ptr<renderer::Mesh> quad_;
};

}  // namespace ferrum::gui

#include <glm/gtc/matrix_transform.hpp>

#include "ferrum/core/Platform.hpp"
#include "ferrum/graphics/GraphicsDevice.hpp"
#include "ferrum/gui/CoreRenderer.hpp"
#include "ferrum/renderer/Renderer.hpp"
#include "ferrum/resources/ResourceManager.hpp"

int main() {
    using ferrum::core::Platform;
    using ferrum::graphics::GraphicsAPI;
    using ferrum::graphics::GraphicsDevice;
    using ferrum::resources::ResourceManager;

    constexpr auto graphics_api = GraphicsAPI::OpenGL;
    auto platform = Platform::Create();
    auto window = platform->CreateWindow({.title = "Ferrum Window", .width = 800, .height = 600},
                                         graphics_api);
    auto device = GraphicsDevice::Create(window->GetGraphicsBinding());
    auto core_renderer = device->CreateRenderer();

    ResourceManager resources;
    ferrum::gui::CoreRenderer gui_renderer(*core_renderer, *device, resources,
                                           [&window] { return window->GetFramebufferSize(); });
    const ferrum::gui::Color background_color{.r = 0.12F, .g = 0.2F, .b = 0.32F};
    const auto render_frame = [&] {
        gui_renderer.BeginFrame();
        gui_renderer.Clear(background_color);
        gui_renderer.FillRect({.x = 40.0F, .y = 40.0F, .width = 240.0F, .height = 120.0F},
                              {.r = 0.9F, .g = 0.2F, .b = 0.15F, .a = 1.0F});
        gui_renderer.EndFrame();
    };
    window->SetRefreshCallback(render_frame);

    while (!window->ShouldClose()) {
        window->PollEvents();
        render_frame();
    }
    return 0;
}

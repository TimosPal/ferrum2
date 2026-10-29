#include "ferrum/core/Platform.hpp"
#include "ferrum/renderer/Renderer.hpp"

int main() {
    constexpr auto graphicsAPI = ferrum::graphics::GraphicsAPI::OpenGL;
    auto platform = ferrum::core::Platform::Create();
    auto window = platform->CreateWindow({.title = "Ferrum Window", .width = 800, .height = 600},
                                         graphicsAPI);
    auto renderer = ferrum::renderer::Renderer::Create(window->GetGraphicsBinding());

    const ferrum::graphics::Color backgroundColor{.r = 0.12F, .g = 0.2F, .b = 0.32F};
    const auto renderFrame = [&] {
        renderer->Clear(backgroundColor);
        renderer->Present();
    };
    window->SetRefreshCallback(renderFrame);

    while (!window->ShouldClose()) {
        window->PollEvents();
        renderFrame();
    }
    return 0;
}

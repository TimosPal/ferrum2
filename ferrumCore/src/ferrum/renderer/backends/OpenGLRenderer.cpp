#include "ferrum/renderer/backends/OpenGLRenderer.hpp"

#include <glad/gl.h>

#include <mutex>

#include "ferrum/common/exception.hpp"

namespace ferrum::renderer {
namespace {

std::once_flag gladInitialization;

GLADapiproc LoadOpenGLProcedure(void* binding, const char* name) {
    return static_cast<graphics::OpenGLWindowBinding*>(binding)->GetProcAddress(name);
}

}  // namespace

OpenGLRenderer::OpenGLRenderer(graphics::OpenGLWindowBinding& binding) : binding_(binding) {
    binding_.MakeCurrent();
    std::call_once(gladInitialization, [&] {
        FE_THROW_IF(!gladLoadGLUserPtr(LoadOpenGLProcedure, &binding_),
                    "Failed to initialize GLAD");
    });
}

void OpenGLRenderer::Clear(const graphics::Color& color) {
    binding_.MakeCurrent();
    const auto framebufferSize = binding_.GetFramebufferSize();
    glViewport(0, 0, framebufferSize.x, framebufferSize.y);
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLRenderer::Present() {
    binding_.MakeCurrent();
    binding_.SwapBuffers();
}

}  // namespace ferrum::renderer
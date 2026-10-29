#include "ferrum/renderer/backends/opengl/GLRenderer.hpp"

#include <glad/gl.h>

#include <mutex>

#include "ferrum/common/exception.hpp"
#include "ferrum/renderer/backends/opengl/utils.hpp"

namespace ferrum::renderer::opengl {
namespace {

std::once_flag gladInitialization;

GLADapiproc LoadOpenGLProcedure(void* binding, const char* name) {
    return static_cast<graphics::OpenGLWindowBinding*>(binding)->GetProcAddress(name);
}

}  // namespace

GLRenderer::GLRenderer(graphics::OpenGLWindowBinding& binding) : binding_(binding) {
    binding_.MakeCurrent();
    std::call_once(gladInitialization, [&] {
        FE_THROW_IF(!gladLoadGLUserPtr(LoadOpenGLProcedure, &binding_),
                    "Failed to initialize GLAD");
    });
}

void GLRenderer::Clear(const graphics::Color& color) {
    binding_.MakeCurrent();
    const auto framebufferSize = binding_.GetFramebufferSize();
    FE_GL(glViewport(0, 0, framebufferSize.x, framebufferSize.y));
    FE_GL(glClearColor(color.r, color.g, color.b, color.a));
    FE_GL(glClear(GL_COLOR_BUFFER_BIT));
}

void GLRenderer::Present() {
    binding_.MakeCurrent();
    binding_.SwapBuffers();
}

}  // namespace ferrum::renderer::opengl
#include "ferrum/renderer/Renderer.hpp"

#include "ferrum/common/exception.hpp"
#include "ferrum/graphics/OpenGLWindowBinding.hpp"
#include "ferrum/renderer/backends/opengl/GLRenderer.hpp"

namespace ferrum::renderer {

std::unique_ptr<Renderer> Renderer::Create(graphics::WindowGraphicsBinding* binding) {
    FE_THROW_IF(binding == nullptr, "Cannot create a renderer without a window graphics binding");

    if (auto* open_gl_binding = dynamic_cast<graphics::OpenGLWindowBinding*>(binding)) {
        return std::make_unique<opengl::GLRenderer>(*open_gl_binding);
    }

    FE_THROW("No renderer backend is implemented for the supplied window binding");
}

}  // namespace ferrum::renderer
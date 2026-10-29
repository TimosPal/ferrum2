#include "ferrum/renderer/Renderer.hpp"

#include "ferrum/common/exception.hpp"
#include "ferrum/graphics/OpenGLWindowBinding.hpp"
#include "ferrum/renderer/backends/opengl/GLRenderer.hpp"

namespace ferrum::renderer {

std::unique_ptr<Renderer> Renderer::Create(graphics::WindowGraphicsBinding* binding) {
    FE_THROW_IF(binding == nullptr, "Cannot create a renderer without a window graphics binding");

    if (auto* openGLBinding = dynamic_cast<graphics::OpenGLWindowBinding*>(binding)) {
        return std::make_unique<opengl::GLRenderer>(*openGLBinding);
    }

    FE_THROW("No renderer backend is implemented for the supplied window binding");
}

}  // namespace ferrum::renderer
#include "ferrum/graphics/GraphicsDevice.hpp"

#include "ferrum/common/exception.hpp"
#include "ferrum/graphics/OpenGLWindowBinding.hpp"
#include "ferrum/renderer/backends/opengl/GLGraphicsDevice.hpp"

namespace ferrum::graphics {

std::unique_ptr<GraphicsDevice> GraphicsDevice::Create(WindowGraphicsBinding* binding) {
    FE_THROW_IF(binding == nullptr, "Cannot create a graphics device without a window binding");

    if (auto* open_gl_binding = dynamic_cast<OpenGLWindowBinding*>(binding)) {
        return std::make_unique<renderer::opengl::GLGraphicsDevice>(*open_gl_binding);
    }

    FE_THROW("No graphics device backend is implemented for the supplied window binding");
}

}  // namespace ferrum::graphics

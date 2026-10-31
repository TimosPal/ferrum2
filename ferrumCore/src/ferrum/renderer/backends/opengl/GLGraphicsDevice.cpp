#include "ferrum/renderer/backends/opengl/GLGraphicsDevice.hpp"

#include "ferrum/renderer/backends/opengl/GLProgram.hpp"
#include "ferrum/renderer/backends/opengl/GLRenderer.hpp"
#include "ferrum/renderer/backends/opengl/GLShader.hpp"

namespace ferrum::renderer::opengl {

std::unique_ptr<renderer::Renderer> GLGraphicsDevice::CreateRenderer() {
    return std::make_unique<GLRenderer>(binding_);
}

std::unique_ptr<renderer::ShaderModule> GLGraphicsDevice::CreateShaderModule(
    const renderer::ShaderModuleDesc& description) {
    return std::make_unique<GLShader>(description.type, description.source);
}

std::unique_ptr<renderer::ProgramModule> GLGraphicsDevice::CreateProgramModule(
    const renderer::ProgramModuleDesc& description) {
    return std::make_unique<GLProgram>(description);
}

}  // namespace ferrum::renderer::opengl

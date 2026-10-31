#include "ferrum/renderer/backends/opengl/GLGraphicsDevice.hpp"

#include <vector>

#include "ferrum/common/exception.hpp"
#include "ferrum/renderer/backends/opengl/GLMesh.hpp"
#include "ferrum/renderer/backends/opengl/GLMeshConfig.hpp"
#include "ferrum/renderer/backends/opengl/GLProgram.hpp"
#include "ferrum/renderer/backends/opengl/GLRenderer.hpp"
#include "ferrum/renderer/backends/opengl/GLShader.hpp"
#include "ferrum/renderer/backends/opengl/GLTexture.hpp"

namespace ferrum::renderer::opengl {

namespace {

GLBuffer::Usage ToGLBufferUsage(renderer::BufferUsage usage) {
    switch (usage) {
        case renderer::BufferUsage::Static:
            return GLBuffer::Usage::StaticDraw;
        case renderer::BufferUsage::Dynamic:
            return GLBuffer::Usage::DynamicDraw;
        case renderer::BufferUsage::Stream:
            return GLBuffer::Usage::StreamDraw;
        default:
            FE_THROW("Unsupported buffer usage");
    }
}

}  // namespace

std::unique_ptr<renderer::Renderer> GLGraphicsDevice::CreateRenderer() {
    return std::make_unique<GLRenderer>(binding_);
}

std::unique_ptr<renderer::ShaderModule> GLGraphicsDevice::CreateShaderModule(
    const renderer::ShaderModuleDesc& description) {
    return std::make_unique<GLShader>(description.type, description.source);
}

std::unique_ptr<renderer::ProgramModule> GLGraphicsDevice::CreateProgramModule(
    const renderer::ProgramModuleDesc& description) {
    const auto* vertex_shader = dynamic_cast<const GLShader*>(&description.vertex_shader);
    const auto* fragment_shader = dynamic_cast<const GLShader*>(&description.fragment_shader);
    FE_THROW_IF(vertex_shader == nullptr || fragment_shader == nullptr,
                "OpenGL program requires OpenGL shader modules");
    return std::make_unique<GLProgram>(*vertex_shader, *fragment_shader);
}

std::unique_ptr<renderer::Mesh> GLGraphicsDevice::CreateMesh(
    const renderer::MeshDesc& description) {
    const auto usage = ToGLBufferUsage(description.usage);
    std::vector<GLMeshVertexBufferConfig> vertex_buffers;
    vertex_buffers.reserve(description.vertex_buffers.size());
    for (const auto& vertex_buffer : description.vertex_buffers) {
        vertex_buffers.emplace_back(vertex_buffer, usage);
    }
    const GLMeshIndexBufferConfig index_buffer(description.index_buffer, usage);
    return std::make_unique<GLMesh>(vertex_buffers, index_buffer);
}

std::unique_ptr<renderer::Texture> GLGraphicsDevice::CreateTexture(
    const renderer::TextureDesc& description, renderer::TextureOptions options) {
    return std::make_unique<GLTexture>(description, options);
}

}  // namespace ferrum::renderer::opengl

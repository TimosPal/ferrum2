#include "ferrum/renderer/backends/opengl/GLRenderer.hpp"

#include <glad/gl.h>

#include <mutex>

#include "ferrum/common/exception.hpp"
#include "ferrum/renderer/backends/opengl/GLMesh.hpp"
#include "ferrum/renderer/backends/opengl/GLProgram.hpp"
#include "ferrum/renderer/backends/opengl/utils.hpp"

namespace ferrum::renderer::opengl {
namespace {

std::once_flag glad_initialization;

GLADapiproc LoadOpenGLProcedure(void* binding, const char* name) {
    return static_cast<graphics::OpenGLWindowBinding*>(binding)->GetProcAddress(name);
}

GLenum ToGLPrimitiveTopology(renderer::PrimitiveTopology topology) {
    switch (topology) {
        case renderer::PrimitiveTopology::Triangles:
            return GL_TRIANGLES;
        case renderer::PrimitiveTopology::Lines:
            return GL_LINES;
        case renderer::PrimitiveTopology::Points:
            return GL_POINTS;
        default:
            FE_THROW("Unsupported primitive topology");
    }
}

}  // namespace

GLRenderer::GLRenderer(graphics::OpenGLWindowBinding& binding) : binding_(binding) {
    binding_.MakeCurrent();
    std::call_once(glad_initialization, [&] {
        FE_THROW_IF(!gladLoadGLUserPtr(LoadOpenGLProcedure, &binding_),
                    "Failed to initialize GLAD");
    });
}

void GLRenderer::BeginFrame(const renderer::CameraMatrices& camera_matrices) {
    binding_.MakeCurrent();
    camera_matrices_ = &camera_matrices;
}

void GLRenderer::Clear(const graphics::Color& color) {
    binding_.MakeCurrent();
    const auto framebuffer_size = binding_.GetFramebufferSize();
    FE_GL(glViewport(0, 0, framebuffer_size.x, framebuffer_size.y));
    FE_GL(glClearColor(color.r, color.g, color.b, color.a));
    FE_GL(glClear(GL_COLOR_BUFFER_BIT));
}

void GLRenderer::DrawProgram(const renderer::ProgramModule& program, renderer::Mesh& mesh,
                             renderer::PrimitiveTopology topology) {
    DrawProgram(program, mesh, mesh.IndexCount(), 0, topology);
}

void GLRenderer::DrawProgram(const renderer::ProgramModule& program, renderer::Mesh& mesh,
                             std::size_t index_count, std::size_t index_offset,
                             renderer::PrimitiveTopology topology) {
    binding_.MakeCurrent();
    const auto* open_gl_program = dynamic_cast<const GLProgram*>(&program);
    auto* open_gl_mesh = dynamic_cast<GLMesh*>(&mesh);
    FE_THROW_IF(open_gl_program == nullptr || open_gl_mesh == nullptr,
                "OpenGL renderer requires OpenGL program and mesh resources");
    open_gl_program->Use();
    open_gl_mesh->ValidateDrawRange(index_count, index_offset);
    open_gl_mesh->vertex_array_.Bind();
    FE_GL(glDrawElements(
        ToGLPrimitiveTopology(topology), static_cast<GLsizei>(index_count),
        open_gl_mesh->index_buffer_.index_type,
        reinterpret_cast<void*>(index_offset * open_gl_mesh->index_buffer_.index_size_bytes)));
}

void GLRenderer::Draw(renderer::Material& material, renderer::Mesh& mesh,
                      renderer::PrimitiveTopology topology) {
    Draw(material, mesh, mesh.IndexCount(), 0, topology);
}

void GLRenderer::Draw(renderer::Material& material, renderer::Mesh& mesh, std::size_t index_count,
                      std::size_t index_offset, renderer::PrimitiveTopology topology) {
    binding_.MakeCurrent();
    FE_THROW_IF(camera_matrices_ == nullptr, "Cannot draw without an active camera frame");
    material.GetProgram().SetParameter("viewProjection", camera_matrices_->view_projection);
    material.Apply();
    DrawProgram(material.GetProgram(), mesh, index_count, index_offset, topology);
}

void GLRenderer::EndFrame() {
    binding_.MakeCurrent();
    camera_matrices_ = nullptr;
    binding_.SwapBuffers();
}

}  // namespace ferrum::renderer::opengl
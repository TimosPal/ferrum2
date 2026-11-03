#pragma once

#include <cstddef>
#include <glm/mat4x4.hpp>

#include "ferrum/graphics/Color.hpp"
#include "ferrum/renderer/CameraMatrices.hpp"
#include "ferrum/renderer/Material.hpp"
#include "ferrum/renderer/Mesh.hpp"
#include "ferrum/renderer/ProgramModule.hpp"

namespace ferrum::renderer {

class Renderer {
public:
    virtual ~Renderer() = default;
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    virtual void BeginFrame(const CameraMatrices& camera_matrices) = 0;
    virtual void Clear(const graphics::Color& color) = 0;
    // Model and viewProjection are renderer-owned shader uniforms.
    virtual void Draw(Material& material, Mesh& mesh, const glm::mat4& model = glm::mat4(1.0F),
                      PrimitiveTopology topology = PrimitiveTopology::Triangles) = 0;
    virtual void Draw(Material& material, Mesh& mesh, std::size_t index_count,
                      std::size_t index_offset, const glm::mat4& model = glm::mat4(1.0F),
                      PrimitiveTopology topology = PrimitiveTopology::Triangles) = 0;
    virtual void EndFrame() = 0;

protected:
    Renderer() = default;
};

}  // namespace ferrum::renderer

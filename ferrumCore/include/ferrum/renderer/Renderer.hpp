#pragma once

#include <cstddef>

#include "ferrum/graphics/Color.hpp"
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

    virtual void Clear(const graphics::Color& color) = 0;
    virtual void Draw(const ProgramModule& program, Mesh& mesh,
                      PrimitiveTopology topology = PrimitiveTopology::Triangles) = 0;
    virtual void Draw(const ProgramModule& program, Mesh& mesh, std::size_t index_count,
                      std::size_t index_offset,
                      PrimitiveTopology topology = PrimitiveTopology::Triangles) = 0;
    virtual void Present() = 0;

protected:
    Renderer() = default;
};

}  // namespace ferrum::renderer
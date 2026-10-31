#pragma once

#include <memory>

#include "ferrum/renderer/Mesh.hpp"
#include "ferrum/renderer/ProgramModule.hpp"
#include "ferrum/renderer/ShaderModule.hpp"

namespace ferrum::graphics {

class WindowGraphicsBinding;
}

namespace ferrum::renderer {

class Renderer;
}

namespace ferrum::graphics {

class GraphicsDevice {
public:
    static std::unique_ptr<GraphicsDevice> Create(WindowGraphicsBinding* binding);

    virtual ~GraphicsDevice() = default;
    GraphicsDevice(const GraphicsDevice&) = delete;
    GraphicsDevice& operator=(const GraphicsDevice&) = delete;
    GraphicsDevice(GraphicsDevice&&) = delete;
    GraphicsDevice& operator=(GraphicsDevice&&) = delete;

    virtual std::unique_ptr<renderer::Renderer> CreateRenderer() = 0;
    virtual std::unique_ptr<renderer::ShaderModule> CreateShaderModule(
        const renderer::ShaderModuleDesc& description) = 0;
    virtual std::unique_ptr<renderer::ProgramModule> CreateProgramModule(
        const renderer::ProgramModuleDesc& description) = 0;
    virtual std::unique_ptr<renderer::Mesh> CreateMesh(const renderer::MeshDesc& description) = 0;

protected:
    GraphicsDevice() = default;
};

}  // namespace ferrum::graphics

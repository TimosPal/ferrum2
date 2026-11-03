#pragma once

#include <memory>

#include "ferrum/renderer/ProgramModule.hpp"
#include "ferrum/renderer/ShaderModule.hpp"
#include "ferrum/renderer/Texture.hpp"
#include "ferrum/resources/FactoryTraits.hpp"
#include "ferrum/resources/ResourceHandle.hpp"
#include "ferrum/resources/types/ImageResource.hpp"
#include "ferrum/resources/types/TextResource.hpp"

namespace ferrum::graphics {

class GraphicsDevice;

}  // namespace ferrum::graphics

namespace ferrum::resources {

struct ShaderFactoryDesc {
    graphics::GraphicsDevice& device;
    renderer::ShaderType type;
    ResourceHandle<types::TextResource> source;
};

struct ProgramFactoryDesc {
    graphics::GraphicsDevice& device;
    ResourceHandle<renderer::ShaderModule> vertex_shader;
    ResourceHandle<renderer::ShaderModule> fragment_shader;
};

struct TextureFactoryDesc {
    graphics::GraphicsDevice& device;
    ResourceHandle<types::ImageResource> image;
    renderer::TextureOptions options = {};
};

template <>
struct FactoryTraits<renderer::ShaderModule> {
    using Description = ShaderFactoryDesc;
};

template <>
struct FactoryTraits<renderer::ProgramModule> {
    using Description = ProgramFactoryDesc;
};

template <>
struct FactoryTraits<renderer::Texture> {
    using Description = TextureFactoryDesc;
};

namespace factories {

std::unique_ptr<renderer::ShaderModule> CreateShaderModule(ShaderFactoryDesc& description);
std::unique_ptr<renderer::ProgramModule> CreateProgramModule(ProgramFactoryDesc& description);
std::unique_ptr<renderer::Texture> CreateTexture(TextureFactoryDesc& description);

}  // namespace factories
}  // namespace ferrum::resources

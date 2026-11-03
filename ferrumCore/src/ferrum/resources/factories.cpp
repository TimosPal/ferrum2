#include "ferrum/resources/factories.hpp"

#include <memory>

#include "ferrum/common/exception.hpp"
#include "ferrum/graphics/GraphicsDevice.hpp"

namespace ferrum::resources::factories {

std::unique_ptr<renderer::ShaderModule> CreateShaderModule(ShaderFactoryDesc& description) {
    FE_THROW_IF(!description.source.IsValid(), "Shader resource requires a source handle");
    return description.device.CreateShaderModule({
        .type = description.type,
        .source = description.source.GetResource()->GetText(),
    });
}

std::unique_ptr<renderer::ProgramModule> CreateProgramModule(ProgramFactoryDesc& description) {
    FE_THROW_IF(!description.vertex_shader.IsValid() || !description.fragment_shader.IsValid(),
                "Program resource requires vertex and fragment shader handles");
    return description.device.CreateProgramModule({
        .vertex_shader = *description.vertex_shader.GetResource(),
        .fragment_shader = *description.fragment_shader.GetResource(),
    });
}

std::unique_ptr<renderer::Texture> CreateTexture(TextureFactoryDesc& description) {
    FE_THROW_IF(!description.image.IsValid(), "Texture resource requires an image handle");
    const auto& image = *description.image.GetResource();
    auto texture = description.device.CreateTexture(
        {
            .width = image.GetWidth(),
            .height = image.GetHeight(),
            .channels = image.GetChannels(),
            .data_type = image.GetDataType(),
        },
        description.options);
    FE_THROW_IF(texture == nullptr, "Graphics device returned a null texture");
    texture->Upload(image);
    return texture;
}

}  // namespace ferrum::resources::factories

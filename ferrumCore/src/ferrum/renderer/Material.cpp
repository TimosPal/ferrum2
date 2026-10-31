#include "ferrum/renderer/Material.hpp"

#include <utility>

namespace ferrum::renderer {

void Material::SetParameter(std::string_view name, ProgramParameterValue value) {
    parameters_[std::string(name)] = std::move(value);
}

void Material::SetTexture(std::string_view sampler_name, const Texture& texture) {
    auto [binding, inserted] = textures_.try_emplace(
        std::string(sampler_name), TextureBinding{.texture = &texture, .texture_unit = 0});
    if (inserted) {
        binding->second.texture_unit = static_cast<unsigned int>(textures_.size() - 1);
    } else {
        binding->second.texture = &texture;
    }
}

void Material::Apply() {
    for (const auto& [name, value] : parameters_) {
        program_.SetParameter(name, value);
    }
    for (const auto& [sampler_name, binding] : textures_) {
        binding.texture->Bind(binding.texture_unit);
        program_.SetParameter(sampler_name, static_cast<int>(binding.texture_unit));
    }
}

}  // namespace ferrum::renderer

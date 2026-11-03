#include "ferrum/renderer/Material.hpp"

#include <utility>

#include "ferrum/common/exception.hpp"

namespace ferrum::renderer {

Material::Material(resources::ResourceHandle<ProgramModule> program,
                   std::unordered_map<std::string, resources::ResourceHandle<Texture>> textures)
    : program_(std::move(program)), textures_(std::move(textures)) {
    FE_THROW_IF(!program_.IsValid(), "Material requires a valid program handle");
    for (const auto& [sampler_name, texture] : textures_) {
        FE_THROW_IF(!texture.IsValid(), "Material texture '{}' requires a valid handle",
                    sampler_name);
    }
}

void Material::SetParameter(std::string_view name, ProgramParameterValue value) {
    parameters_[std::string(name)] = std::move(value);
}

void Material::Apply() {
    auto& program = GetProgram();
    for (const auto& [name, value] : parameters_) {
        program.SetParameter(name, value);
    }
    unsigned int texture_unit = 0;
    for (auto& [sampler_name, texture] : textures_) {
        texture.GetResource()->Bind(texture_unit);
        program.SetParameter(sampler_name, static_cast<int>(texture_unit));
        ++texture_unit;
    }
}

}  // namespace ferrum::renderer

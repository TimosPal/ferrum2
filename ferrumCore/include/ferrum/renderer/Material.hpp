#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

#include "ferrum/renderer/ProgramModule.hpp"
#include "ferrum/renderer/Texture.hpp"
#include "ferrum/resources/ResourceHandle.hpp"

namespace ferrum::renderer {

class Material {
public:
    // The resource manager must outlive the material.
    Material(resources::ResourceHandle<ProgramModule> program,
             std::unordered_map<std::string, resources::ResourceHandle<Texture>> textures);

    ProgramModule& GetProgram() { return *program_.GetResource(); }
    const ProgramModule& GetProgram() const { return *program_.GetResource(); }

    void SetParameter(std::string_view name, ProgramParameterValue value);
    void Apply();

private:
    mutable resources::ResourceHandle<ProgramModule> program_;
    std::unordered_map<std::string, ProgramParameterValue> parameters_;
    std::unordered_map<std::string, resources::ResourceHandle<Texture>> textures_;
};

}  // namespace ferrum::renderer

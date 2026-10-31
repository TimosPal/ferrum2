#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

#include "ferrum/renderer/ProgramModule.hpp"
#include "ferrum/renderer/Texture.hpp"

namespace ferrum::renderer {

class Material {
public:
    explicit Material(ProgramModule& program) : program_(program) {}

    ProgramModule& GetProgram() noexcept { return program_; }
    const ProgramModule& GetProgram() const noexcept { return program_; }

    void SetParameter(std::string_view name, ProgramParameterValue value);
    void SetTexture(std::string_view sampler_name, const Texture& texture);
    void Apply();

private:
    struct TextureBinding {
        const Texture* texture;
        unsigned int texture_unit;
    };

    ProgramModule& program_;
    std::unordered_map<std::string, ProgramParameterValue> parameters_;
    std::unordered_map<std::string, TextureBinding> textures_;
};

}  // namespace ferrum::renderer

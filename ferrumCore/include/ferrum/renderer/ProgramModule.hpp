#pragma once

#include <glm/glm.hpp>
#include <string_view>
#include <variant>

#include "ferrum/renderer/ShaderModule.hpp"

namespace ferrum::renderer {

using ProgramParameterValue = std::variant<float, int, glm::vec2, glm::vec3, glm::vec4, glm::mat4>;

struct ProgramModuleDesc {
    const ShaderModule& vertex_shader;
    const ShaderModule& fragment_shader;
};

class ProgramModule {
public:
    virtual ~ProgramModule() = default;
    ProgramModule(const ProgramModule&) = delete;
    ProgramModule& operator=(const ProgramModule&) = delete;
    ProgramModule(ProgramModule&&) noexcept = default;
    ProgramModule& operator=(ProgramModule&&) noexcept = default;

    virtual void SetParameter(std::string_view name, const ProgramParameterValue& value) = 0;

protected:
    ProgramModule() = default;
};

}  // namespace ferrum::renderer

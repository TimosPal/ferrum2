#pragma once

#include "ferrum/renderer/ShaderModule.hpp"

namespace ferrum::renderer {

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

protected:
    ProgramModule() = default;
};

}  // namespace ferrum::renderer

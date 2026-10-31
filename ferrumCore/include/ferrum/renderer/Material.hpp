#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

#include "ferrum/renderer/ProgramModule.hpp"

namespace ferrum::renderer {

class Material {
public:
    explicit Material(ProgramModule& program) : program_(program) {}

    ProgramModule& GetProgram() noexcept { return program_; }
    const ProgramModule& GetProgram() const noexcept { return program_; }

    void SetParameter(std::string_view name, ProgramParameterValue value);
    void Apply();

private:
    ProgramModule& program_;
    std::unordered_map<std::string, ProgramParameterValue> parameters_;
};

}  // namespace ferrum::renderer

#include "ferrum/renderer/Material.hpp"

#include <utility>

namespace ferrum::renderer {

void Material::SetParameter(std::string_view name, ProgramParameterValue value) {
    parameters_[std::string(name)] = std::move(value);
}

void Material::Apply() {
    for (const auto& [name, value] : parameters_) {
        program_.SetParameter(name, value);
    }
}

}  // namespace ferrum::renderer

#pragma once

#include <string_view>

#include "ferrum/resources/Resource.hpp"

namespace ferrum::renderer {

enum class ShaderType {
    Vertex,
    Fragment,
};

struct ShaderModuleDesc {
    ShaderType type;
    std::string_view source;
};

class ShaderModule : public resources::Resource {
public:
    ~ShaderModule() override = default;
    ShaderModule(const ShaderModule&) = delete;
    ShaderModule& operator=(const ShaderModule&) = delete;
    ShaderModule(ShaderModule&&) noexcept = default;
    ShaderModule& operator=(ShaderModule&&) noexcept = default;

protected:
    ShaderModule() = default;
};

}  // namespace ferrum::renderer

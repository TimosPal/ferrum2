#pragma once

#include <string_view>

namespace ferrum::renderer {

enum class ShaderType {
    Vertex,
    Fragment,
};

struct ShaderModuleDesc {
    ShaderType type;
    std::string_view source;
};

class ShaderModule {
public:
    virtual ~ShaderModule() = default;
    ShaderModule(const ShaderModule&) = delete;
    ShaderModule& operator=(const ShaderModule&) = delete;
    ShaderModule(ShaderModule&&) noexcept = default;
    ShaderModule& operator=(ShaderModule&&) noexcept = default;

protected:
    ShaderModule() = default;
};

}  // namespace ferrum::renderer

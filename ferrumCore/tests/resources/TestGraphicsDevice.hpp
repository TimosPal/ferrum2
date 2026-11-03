#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ferrum/common/exception.hpp"
#include "ferrum/graphics/GraphicsDevice.hpp"
#include "ferrum/renderer/Renderer.hpp"
#include "ferrum/resources/ResourceManager.hpp"

namespace ferrum::resources::testing {

class TestShaderModule final : public renderer::ShaderModule {
public:
    explicit TestShaderModule(int& destruction_count) : destruction_count_(destruction_count) {}
    ~TestShaderModule() override { ++destruction_count_; }

private:
    int& destruction_count_;
};

class TestProgramModule final : public renderer::ProgramModule {
public:
    explicit TestProgramModule(int& destruction_count) : destruction_count_(destruction_count) {}
    ~TestProgramModule() override { ++destruction_count_; }

    void SetParameter(std::string_view, const renderer::ProgramParameterValue&) override {}

private:
    int& destruction_count_;
};

struct TextureTestState {
    int creation_count = 0;
    int destruction_count = 0;
    int upload_count = 0;
    renderer::TextureDesc description;
    renderer::TextureOptions options;
    std::vector<std::uint8_t> pixels;
    bool fail_creation = false;
    bool return_null = false;
    bool fail_upload = false;
};

class TestTexture final : public renderer::Texture {
public:
    TestTexture(TextureTestState& state, renderer::TextureDesc description)
        : state_(state), description_(description) {}
    ~TestTexture() override { ++state_.destruction_count; }

    int Width() const noexcept override { return description_.width; }
    int Height() const noexcept override { return description_.height; }
    int MipmapLevels() const noexcept override { return 1; }
    void Bind(unsigned int) const override {}

    void Upload(const types::ImageResource& image) override {
        ++state_.upload_count;
        FE_THROW_IF(state_.fail_upload, "Test texture upload failed");
        state_.pixels.assign(image.GetPixels().begin(), image.GetPixels().end());
    }

    void UploadRegion(const types::ImageResource&, glm::ivec2) override {
        FE_THROW("Unexpected texture region upload");
    }

private:
    TextureTestState& state_;
    renderer::TextureDesc description_;
};

class TestGraphicsDevice final : public graphics::GraphicsDevice {
public:
    std::unique_ptr<renderer::ShaderModule> CreateShaderModule(
        const renderer::ShaderModuleDesc& description) override {
        ++compilation_count;
        last_type = description.type;
        last_source = description.source;
        FE_THROW_IF(fail_compilation, "Test shader compilation failed");
        if (return_null) {
            return nullptr;
        }
        return std::make_unique<TestShaderModule>(destruction_count);
    }

    std::unique_ptr<renderer::ProgramModule> CreateProgramModule(
        const renderer::ProgramModuleDesc& description) override {
        ++link_count;
        last_vertex_shader = &description.vertex_shader;
        last_fragment_shader = &description.fragment_shader;
        FE_THROW_IF(fail_linking, "Test program linking failed");
        if (return_null_program) {
            return nullptr;
        }
        return std::make_unique<TestProgramModule>(program_destruction_count);
    }

    std::unique_ptr<renderer::Renderer> CreateRenderer() override {
        FE_THROW("Unexpected renderer creation");
    }
    std::unique_ptr<renderer::Mesh> CreateMesh(const renderer::MeshDesc&) override {
        FE_THROW("Unexpected mesh creation");
    }
    std::unique_ptr<renderer::Texture> CreateTexture(const renderer::TextureDesc& description,
                                                     renderer::TextureOptions options) override {
        ++texture.creation_count;
        texture.description = description;
        texture.options = options;
        FE_THROW_IF(texture.fail_creation, "Test texture creation failed");
        if (texture.return_null) {
            return nullptr;
        }
        return std::make_unique<TestTexture>(texture, description);
    }

    int compilation_count = 0;
    int destruction_count = 0;
    renderer::ShaderType last_type = renderer::ShaderType::Vertex;
    std::string last_source;
    bool fail_compilation = false;
    bool return_null = false;
    int link_count = 0;
    int program_destruction_count = 0;
    const renderer::ShaderModule* last_vertex_shader = nullptr;
    const renderer::ShaderModule* last_fragment_shader = nullptr;
    bool fail_linking = false;
    bool return_null_program = false;
    TextureTestState texture;
};

inline auto CreateDeferredShader(ResourceManager& manager, TestGraphicsDevice& device,
                                 renderer::ShaderType type,
                                 ResourceHandle<types::TextResource> source) {
    return manager.CreateDeferredResource<renderer::ShaderModule>({
        .device = device,
        .type = type,
        .source = std::move(source),
    });
}

inline auto CreateDeferredProgram(ResourceManager& manager, TestGraphicsDevice& device,
                                  ResourceHandle<renderer::ShaderModule> vertex,
                                  ResourceHandle<renderer::ShaderModule> fragment) {
    return manager.CreateDeferredResource<renderer::ProgramModule>({
        .device = device,
        .vertex_shader = std::move(vertex),
        .fragment_shader = std::move(fragment),
    });
}

}  // namespace ferrum::resources::testing

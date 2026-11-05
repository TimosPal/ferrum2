#include <gtest/gtest.h>

#include <limits>
#include <memory>
#include <variant>

#include "ferrum/common/exception.hpp"
#include "ferrum/graphics/GraphicsDevice.hpp"
#include "ferrum/gui/CoreRenderer.hpp"
#include "ferrum/renderer/Renderer.hpp"
#include "ferrum/resources/ResourceManager.hpp"

namespace ferrum::gui {
namespace {

struct TestState {
    int shader_creations = 0;
    int program_creations = 0;
    int mesh_creations = 0;
    bool null_mesh = false;
    bool fail_shader = false;
    glm::vec4 color{};
};

class TestShader final : public renderer::ShaderModule {};

class TestProgram final : public renderer::ProgramModule {
public:
    explicit TestProgram(TestState& state) : state_(state) {}
    void SetParameter(std::string_view name,
                      const renderer::ProgramParameterValue& value) override {
        FE_THROW_IF(name != "color", "Unexpected test program parameter");
        state_.color = std::get<glm::vec4>(value);
    }

private:
    TestState& state_;
};

class TestMesh final : public renderer::Mesh {
public:
    std::size_t IndexCount() const noexcept override { return 6; }
};

class TestDevice final : public graphics::GraphicsDevice {
public:
    explicit TestDevice(TestState& state) : state_(state) {}
    std::unique_ptr<renderer::Renderer> CreateRenderer() override {
        FE_THROW("Unexpected renderer creation");
    }
    std::unique_ptr<renderer::ShaderModule> CreateShaderModule(
        const renderer::ShaderModuleDesc&) override {
        ++state_.shader_creations;
        FE_THROW_IF(state_.fail_shader, "Test shader compilation failed");
        return std::make_unique<TestShader>();
    }
    std::unique_ptr<renderer::ProgramModule> CreateProgramModule(
        const renderer::ProgramModuleDesc&) override {
        ++state_.program_creations;
        return std::make_unique<TestProgram>(state_);
    }
    std::unique_ptr<renderer::Mesh> CreateMesh(const renderer::MeshDesc& description) override {
        ++state_.mesh_creations;
        EXPECT_EQ(description.vertex_buffers.size(), 1);
        EXPECT_EQ(description.vertex_buffers.front().VertexCount(), 4);
        EXPECT_EQ(description.index_buffer.IndexCount(), 6);
        if (state_.null_mesh) {
            return nullptr;
        }
        return std::make_unique<TestMesh>();
    }
    std::unique_ptr<renderer::Texture> CreateTexture(const renderer::TextureDesc&,
                                                     renderer::TextureOptions) override {
        FE_THROW("Unexpected texture creation");
    }

private:
    TestState& state_;
};

class TestRenderer final : public renderer::Renderer {
public:
    void BeginFrame(const renderer::CameraMatrices& camera_matrices) override {
        ++frame_operations;
        last_camera = camera_matrices;
    }
    void Clear(const graphics::Color& color) override {
        ++frame_operations;
        last_clear = color;
    }
    void EndFrame() override { ++frame_operations; }
    void Draw(renderer::Material& material, renderer::Mesh& mesh, const glm::mat4& model,
              renderer::PrimitiveTopology topology) override {
        EXPECT_EQ(topology, renderer::PrimitiveTopology::Triangles);
        material.Apply();
        last_model = model;
        last_mesh = &mesh;
        last_program = &material.GetProgram();
        ++draws;
    }
    void Draw(renderer::Material&, renderer::Mesh&, std::size_t, std::size_t, const glm::mat4&,
              renderer::PrimitiveTopology) override {
        FE_THROW("Unexpected ranged draw");
    }

    int frame_operations = 0;
    int draws = 0;
    renderer::CameraMatrices last_camera;
    graphics::Color last_clear;
    glm::mat4 last_model{1.0F};
    renderer::Mesh* last_mesh = nullptr;
    renderer::ProgramModule* last_program = nullptr;
};

class CoreRendererTest : public ::testing::Test {
protected:
    std::function<glm::ivec2()> FramebufferSizeProvider() {
        return [this] { return framebuffer_size; };
    }

    TestState state;
    TestDevice device{state};
    resources::ResourceManager resources;
    TestRenderer renderer;
    glm::ivec2 framebuffer_size{800, 600};
};

TEST_F(CoreRendererTest, GraphicsColorValidityRequiresFiniteUnitComponents) {
    const graphics::Color valid_color{0.0F, 0.5F, 1.0F, 1.0F};
    const graphics::Color negative_color{-0.1F, 0.0F, 0.0F, 1.0F};
    const graphics::Color oversized_color{0.0F, 1.1F, 0.0F, 1.0F};
    const graphics::Color nan_color{0.0F, 0.0F, 0.0F, std::numeric_limits<float>::quiet_NaN()};
    EXPECT_TRUE(valid_color.IsValid());
    EXPECT_FALSE(negative_color.IsValid());
    EXPECT_FALSE(oversized_color.IsValid());
    EXPECT_FALSE(nan_color.IsValid());
}

TEST_F(CoreRendererTest, MapsTopLeftBoundsToSharedCenteredQuadAndForwardsColor) {
    CoreRenderer adapter(renderer, device, resources, FramebufferSizeProvider());
    EXPECT_EQ(state.shader_creations, 0);
    ASSERT_TRUE(adapter.BeginFrame());
    EXPECT_EQ(renderer.last_camera.view_projection * glm::vec4(0.0F, 0.0F, 0.0F, 1.0F),
              glm::vec4(-1.0F, 1.0F, 0.0F, 1.0F));
    EXPECT_EQ(renderer.last_camera.view_projection * glm::vec4(800.0F, 600.0F, 0.0F, 1.0F),
              glm::vec4(1.0F, -1.0F, 0.0F, 1.0F));
    adapter.FillRect({10.0F, 20.0F, 30.0F, 40.0F}, {1.0F, 0.25F, 0.5F, 0.5F});
    EXPECT_EQ(renderer.last_model * glm::vec4(-0.5F, -0.5F, 0.0F, 1.0F),
              glm::vec4(10.0F, 20.0F, 0.0F, 1.0F));
    EXPECT_EQ(renderer.last_model * glm::vec4(0.5F, 0.5F, 0.0F, 1.0F),
              glm::vec4(40.0F, 60.0F, 0.0F, 1.0F));
    EXPECT_EQ(state.color, glm::vec4(1.0F, 0.25F, 0.5F, 0.5F));
    EXPECT_EQ(renderer.draws, 1);
    adapter.EndFrame();
    EXPECT_EQ(renderer.frame_operations, 2);
}

TEST_F(CoreRendererTest, ReusesMeshAndProgramAcrossDifferentRectangles) {
    CoreRenderer adapter(renderer, device, resources, FramebufferSizeProvider());
    ASSERT_TRUE(adapter.BeginFrame());
    adapter.FillRect({0.0F, 0.0F, 10.0F, 10.0F}, {1.0F, 0.0F, 0.0F, 1.0F});
    auto* mesh = renderer.last_mesh;
    auto* program = renderer.last_program;
    adapter.FillRect({50.0F, 60.0F, 100.0F, 200.0F}, {0.0F, 1.0F, 0.0F, 0.25F});
    EXPECT_EQ(renderer.last_mesh, mesh);
    EXPECT_EQ(renderer.last_program, program);
    EXPECT_EQ(state.mesh_creations, 1);
    EXPECT_EQ(state.shader_creations, 2);
    EXPECT_EQ(state.program_creations, 1);
    EXPECT_EQ(state.color, glm::vec4(0.0F, 1.0F, 0.0F, 0.25F));
    EXPECT_EQ(renderer.draws, 2);
    adapter.EndFrame();
}

TEST_F(CoreRendererTest, EmptyRectanglesDoNotDrawOrCompileShaders) {
    CoreRenderer adapter(renderer, device, resources, FramebufferSizeProvider());
    ASSERT_TRUE(adapter.BeginFrame());
    adapter.FillRect({0.0F, 0.0F, 0.0F, 10.0F}, {});
    adapter.FillRect({0.0F, 0.0F, 10.0F, 0.0F}, {});
    EXPECT_EQ(renderer.draws, 0);
    EXPECT_EQ(state.shader_creations, 0);
    adapter.EndFrame();
}

TEST_F(CoreRendererTest, RejectsInvalidBoundsBeforeDrawing) {
    CoreRenderer adapter(renderer, device, resources, FramebufferSizeProvider());
    const float infinity = std::numeric_limits<float>::infinity();
    const float nan = std::numeric_limits<float>::quiet_NaN();
    const float maximum = std::numeric_limits<float>::max();
    for (const Rect bounds :
         {Rect{0.0F, 0.0F, -1.0F, 10.0F}, Rect{0.0F, 0.0F, 10.0F, -1.0F},
          Rect{infinity, 0.0F, 10.0F, 10.0F}, Rect{0.0F, nan, 10.0F, 10.0F},
          Rect{0.0F, 0.0F, infinity, 10.0F}, Rect{maximum, 0.0F, maximum, 10.0F}}) {
        EXPECT_THROW(adapter.FillRect(bounds, {}), common::Exception);
    }
    EXPECT_EQ(renderer.draws, 0);
}

TEST_F(CoreRendererTest, RejectsInvalidColorsBeforeDrawing) {
    CoreRenderer adapter(renderer, device, resources, FramebufferSizeProvider());
    ASSERT_TRUE(adapter.BeginFrame());
    for (const Color color : {Color{-0.1F, 0.0F, 0.0F, 1.0F}, Color{0.0F, 1.1F, 0.0F, 1.0F},
                              Color{0.0F, 0.0F, 0.0F, 2.0F},
                              Color{0.0F, 0.0F, std::numeric_limits<float>::quiet_NaN(), 1.0F}}) {
        EXPECT_THROW(adapter.FillRect({0.0F, 0.0F, 10.0F, 10.0F}, color), common::Exception);
    }
    EXPECT_EQ(renderer.draws, 0);
    adapter.EndFrame();
}

TEST_F(CoreRendererTest, SkipsFramesWithoutDrawableFramebufferSize) {
    CoreRenderer adapter(renderer, device, resources, FramebufferSizeProvider());
    framebuffer_size = {0, 600};
    EXPECT_FALSE(adapter.BeginFrame());
    framebuffer_size = {800, -1};
    EXPECT_FALSE(adapter.BeginFrame());
    EXPECT_EQ(renderer.frame_operations, 0);

    framebuffer_size = {1024, 768};
    ASSERT_TRUE(adapter.BeginFrame());
    EXPECT_EQ(renderer.last_camera.view_projection * glm::vec4(1024.0F, 768.0F, 0.0F, 1.0F),
              glm::vec4(1.0F, -1.0F, 0.0F, 1.0F));
    adapter.EndFrame();
}

TEST_F(CoreRendererTest, ClearForwardsColor) {
    CoreRenderer adapter(renderer, device, resources, FramebufferSizeProvider());
    ASSERT_TRUE(adapter.BeginFrame());
    adapter.Clear({0.1F, 0.2F, 0.3F, 0.4F});
    EXPECT_EQ(glm::vec4(renderer.last_clear.r, renderer.last_clear.g, renderer.last_clear.b,
                        renderer.last_clear.a),
              glm::vec4(0.1F, 0.2F, 0.3F, 0.4F));
    adapter.EndFrame();
}

TEST_F(CoreRendererTest, RejectsNullMeshCreation) {
    state.null_mesh = true;
    EXPECT_THROW((CoreRenderer{renderer, device, resources, FramebufferSizeProvider()}),
                 common::Exception);
}

TEST_F(CoreRendererTest, PropagatesShaderFailuresAndAllowsRetry) {
    CoreRenderer adapter(renderer, device, resources, FramebufferSizeProvider());
    ASSERT_TRUE(adapter.BeginFrame());
    state.fail_shader = true;
    EXPECT_THROW(adapter.FillRect({0.0F, 0.0F, 10.0F, 10.0F}, {}), common::Exception);
    state.fail_shader = false;
    EXPECT_NO_THROW(adapter.FillRect({0.0F, 0.0F, 10.0F, 10.0F}, {}));
    EXPECT_EQ(renderer.draws, 1);
    EXPECT_EQ(state.program_creations, 1);
    EXPECT_EQ(state.mesh_creations, 1);
    adapter.EndFrame();
}

}  // namespace
}  // namespace ferrum::gui

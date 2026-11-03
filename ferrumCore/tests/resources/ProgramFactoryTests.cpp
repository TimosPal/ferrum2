#include <gtest/gtest.h>

#include <filesystem>
#include <type_traits>
#include <utility>

#include "TestGraphicsDevice.hpp"
#include "ferrum/common/exception.hpp"
#include "ferrum/resources/ResourceManager.hpp"

namespace ferrum::resources {
namespace {

using renderer::ProgramModule;
using renderer::ShaderModule;
using renderer::ShaderType;
using testing::CreateDeferredProgram;
using testing::CreateDeferredShader;
using testing::TestGraphicsDevice;
using types::TextResource;

static_assert(std::is_base_of_v<Resource, ProgramModule>);
static_assert(std::is_same_v<decltype(std::declval<ResourceHandle<ProgramModule>&>().GetResource()),
                             ProgramModule*>);

auto CreateShader(ResourceManager& manager, TestGraphicsDevice& device, ShaderType type) {
    auto source = manager.LoadResource<TextResource>(std::filesystem::path(FERRUM_TEST_ASSET_ROOT) /
                                                     "text.txt");
    return CreateDeferredShader(manager, device, type, source.handle);
}

class ProgramFactoryTest : public ::testing::Test {
protected:
    auto CreateProgram() {
        return CreateDeferredProgram(manager, device, vertex_shader.handle, fragment_shader.handle);
    }

    TestGraphicsDevice device;
    ResourceManager manager;
    ManagedResource<ShaderModule> vertex_shader = CreateShader(manager, device, ShaderType::Vertex);
    ManagedResource<ShaderModule> fragment_shader =
        CreateShader(manager, device, ShaderType::Fragment);
};

TEST_F(ProgramFactoryTest, CompilesAndLinksOnlyOnFirstGetResource) {
    auto program = CreateProgram();
    auto retrieved = manager.GetResource<ProgramModule>(program.id);
    EXPECT_EQ(device.compilation_count, 0);
    EXPECT_EQ(device.link_count, 0);

    auto* resource = program.handle.GetResource();
    EXPECT_EQ(device.compilation_count, 2);
    EXPECT_EQ(device.link_count, 1);
    EXPECT_EQ(device.last_source, "Ferrum test resource\n");
    EXPECT_EQ(device.last_vertex_shader, vertex_shader.handle.GetResource());
    EXPECT_EQ(device.last_fragment_shader, fragment_shader.handle.GetResource());
    EXPECT_EQ(retrieved.GetResource(), resource);
    EXPECT_EQ(device.compilation_count, 2);
    EXPECT_EQ(device.link_count, 1);
}

TEST_F(ProgramFactoryTest, SharesCompiledShaderAcrossDifferentPrograms) {
    auto first = CreateProgram();
    auto other_fragment = CreateShader(manager, device, ShaderType::Fragment);
    auto second =
        CreateDeferredProgram(manager, device, vertex_shader.handle, other_fragment.handle);
    auto* first_resource = first.handle.GetResource();
    const auto* shared_vertex = device.last_vertex_shader;
    const auto* first_fragment = device.last_fragment_shader;
    auto* second_resource = second.handle.GetResource();

    EXPECT_NE(first_resource, second_resource);
    EXPECT_EQ(device.last_vertex_shader, shared_vertex);
    EXPECT_NE(device.last_fragment_shader, first_fragment);
    EXPECT_EQ(device.compilation_count, 3);
    EXPECT_EQ(device.link_count, 2);
}

TEST_F(ProgramFactoryTest, ReportsInvalidShaderHandlesOnGetResource) {
    auto program = CreateDeferredProgram(manager, device, ResourceHandle<ShaderModule>(nullptr),
                                         fragment_shader.handle);
    EXPECT_THROW(program.handle.GetResource(), common::Exception);
    EXPECT_EQ(device.compilation_count, 0);
    EXPECT_EQ(device.link_count, 0);
}

TEST_F(ProgramFactoryTest, DefersSourceLoadErrorsUntilGetResource) {
    auto source = manager.LoadResource<TextResource>(std::filesystem::path(FERRUM_TEST_ASSET_ROOT) /
                                                     "missing.txt");
    auto missing_shader = CreateDeferredShader(manager, device, ShaderType::Vertex, source.handle);
    auto program =
        CreateDeferredProgram(manager, device, missing_shader.handle, fragment_shader.handle);
    EXPECT_THROW(program.handle.GetResource(), common::Exception);
    EXPECT_EQ(device.compilation_count, 0);
    EXPECT_EQ(device.link_count, 0);
}

TEST_F(ProgramFactoryTest, PropagatesCompilationErrorsAndAllowsRetry) {
    auto program = CreateProgram();
    device.fail_compilation = true;
    EXPECT_THROW(program.handle.GetResource(), common::Exception);
    EXPECT_EQ(device.compilation_count, 1);
    EXPECT_EQ(device.link_count, 0);

    device.fail_compilation = false;
    EXPECT_NO_THROW(program.handle.GetResource());
    EXPECT_EQ(device.compilation_count, 3);
    EXPECT_EQ(device.link_count, 1);
}

TEST_F(ProgramFactoryTest, PropagatesLinkErrorsAndReusesCompiledShadersOnRetry) {
    auto program = CreateProgram();
    device.fail_linking = true;
    EXPECT_THROW(program.handle.GetResource(), common::Exception);
    EXPECT_EQ(device.compilation_count, 2);
    EXPECT_EQ(device.link_count, 1);

    device.fail_linking = false;
    EXPECT_NO_THROW(program.handle.GetResource());
    EXPECT_EQ(device.compilation_count, 2);
    EXPECT_EQ(device.link_count, 2);
}

TEST_F(ProgramFactoryTest, RejectsNullProgramModuleAndAllowsRetry) {
    auto program = CreateProgram();
    device.return_null_program = true;
    EXPECT_THROW(program.handle.GetResource(), common::Exception);
    device.return_null_program = false;
    EXPECT_NO_THROW(program.handle.GetResource());
    EXPECT_EQ(device.compilation_count, 2);
    EXPECT_EQ(device.link_count, 2);
}

TEST(ProgramFactory, ResourceManagerRetainsModuleUntilDestruction) {
    TestGraphicsDevice device;
    {
        ResourceManager manager;
        ResourceId id;
        renderer::ProgramModule* expected = nullptr;
        {
            auto vertex = CreateShader(manager, device, ShaderType::Vertex);
            auto fragment = CreateShader(manager, device, ShaderType::Fragment);
            auto program = CreateDeferredProgram(manager, device, vertex.handle, fragment.handle);
            id = program.id;
            expected = program.handle.GetResource();
        }
        EXPECT_EQ(device.program_destruction_count, 0);
        auto retrieved = manager.GetResource<ProgramModule>(id);
        EXPECT_EQ(retrieved.GetResource(), expected);
        EXPECT_EQ(device.link_count, 1);
    }
    EXPECT_EQ(device.program_destruction_count, 1);
    EXPECT_EQ(device.destruction_count, 2);
}

TEST(ProgramFactory, DestroyingUnrequestedFactoriesDoesNotCreateGpuObjects) {
    TestGraphicsDevice device;
    {
        ResourceManager manager;
        auto vertex = CreateShader(manager, device, ShaderType::Vertex);
        auto fragment = CreateShader(manager, device, ShaderType::Fragment);
        auto program = CreateDeferredProgram(manager, device, vertex.handle, fragment.handle);
    }
    EXPECT_EQ(device.compilation_count, 0);
    EXPECT_EQ(device.link_count, 0);
    EXPECT_EQ(device.destruction_count, 0);
    EXPECT_EQ(device.program_destruction_count, 0);
}

}  // namespace
}  // namespace ferrum::resources

#include <gtest/gtest.h>

#include <filesystem>
#include <type_traits>
#include <utility>

#include "TestGraphicsDevice.hpp"
#include "ferrum/common/exception.hpp"
#include "ferrum/resources/ResourceManager.hpp"

namespace ferrum::resources {
namespace {

using renderer::ShaderModule;
using renderer::ShaderType;
using testing::CreateDeferredShader;
using testing::TestGraphicsDevice;
using types::TextResource;

static_assert(std::is_base_of_v<Resource, ShaderModule>);
static_assert(std::is_same_v<decltype(std::declval<ResourceHandle<ShaderModule>&>().GetResource()),
                             ShaderModule*>);

TEST(ShaderFactory, LoadsAndCompilesOnlyOnFirstGetResource) {
    TestGraphicsDevice device;
    ResourceManager manager;
    auto source = manager.LoadResource<TextResource>(std::filesystem::path(FERRUM_TEST_ASSET_ROOT) /
                                                     "text.txt");
    auto shader = CreateDeferredShader(manager, device, ShaderType::Fragment, source.handle);
    auto retrieved = manager.GetResource<ShaderModule>(shader.id);
    EXPECT_EQ(device.compilation_count, 0);

    auto* resource = shader.handle.GetResource();
    EXPECT_EQ(device.compilation_count, 1);
    EXPECT_EQ(device.last_type, ShaderType::Fragment);
    EXPECT_EQ(device.last_source, "Ferrum test resource\n");
    EXPECT_EQ(retrieved.GetResource(), resource);
    EXPECT_EQ(device.compilation_count, 1);
}

TEST(ShaderFactory, DefersSourceLoadErrorsUntilGetResource) {
    TestGraphicsDevice device;
    ResourceManager manager;
    auto source = manager.LoadResource<TextResource>(std::filesystem::path(FERRUM_TEST_ASSET_ROOT) /
                                                     "missing.txt");
    auto shader = CreateDeferredShader(manager, device, ShaderType::Vertex, source.handle);
    EXPECT_THROW(shader.handle.GetResource(), common::Exception);
    EXPECT_EQ(device.compilation_count, 0);
}

TEST(ShaderFactory, PropagatesCompilationErrorsAndAllowsRetry) {
    TestGraphicsDevice device;
    ResourceManager manager;
    auto source = manager.CreateResource(TextResource("source"));
    auto shader = CreateDeferredShader(manager, device, ShaderType::Vertex, source.handle);
    device.fail_compilation = true;
    EXPECT_THROW(shader.handle.GetResource(), common::Exception);
    EXPECT_EQ(device.compilation_count, 1);
    device.fail_compilation = false;
    EXPECT_NO_THROW(shader.handle.GetResource());
    EXPECT_EQ(device.compilation_count, 2);
}

TEST(ShaderFactory, RejectsNullModuleAndAllowsRetry) {
    TestGraphicsDevice device;
    ResourceManager manager;
    auto source = manager.CreateResource(TextResource("source"));
    auto shader = CreateDeferredShader(manager, device, ShaderType::Vertex, source.handle);
    device.return_null = true;
    EXPECT_THROW(shader.handle.GetResource(), common::Exception);
    device.return_null = false;
    EXPECT_NO_THROW(shader.handle.GetResource());
    EXPECT_EQ(device.compilation_count, 2);
}

TEST(ShaderFactory, ResourceManagerRetainsModuleUntilDestruction) {
    TestGraphicsDevice device;
    {
        ResourceManager manager;
        ResourceId id;
        const renderer::ShaderModule* expected = nullptr;
        {
            auto source = manager.LoadResource<TextResource>(
                std::filesystem::path(FERRUM_TEST_ASSET_ROOT) / "text.txt");
            auto shader = CreateDeferredShader(manager, device, ShaderType::Vertex, source.handle);
            id = shader.id;
            expected = shader.handle.GetResource();
        }
        EXPECT_EQ(device.destruction_count, 0);
        auto retrieved = manager.GetResource<ShaderModule>(id);
        EXPECT_EQ(retrieved.GetResource(), expected);
        EXPECT_EQ(device.compilation_count, 1);
    }
    EXPECT_EQ(device.destruction_count, 1);
}

}  // namespace
}  // namespace ferrum::resources

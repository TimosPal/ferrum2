#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <utility>

#include "TestGraphicsDevice.hpp"
#include "ferrum/common/exception.hpp"
#include "ferrum/resources/ResourceManager.hpp"

namespace ferrum::resources {
namespace {

using types::TextResource;

class UnregisteredResource final : public Resource {
public:
    using Description = int;
};

TEST(ResourceManager, RetrievesCreatedResourcesById) {
    ResourceManager manager;
    auto text = manager.CreateResource(TextResource("hello"));
    EXPECT_TRUE(manager.HasResource(text.id));
    auto retrieved = manager.GetResource<TextResource>(text.id);
    EXPECT_EQ(retrieved.GetResource(), text.handle.GetResource());
    EXPECT_EQ(retrieved.GetResource()->GetText(), "hello");
    EXPECT_FALSE(manager.HasResource({}));
}

TEST(ResourceManager, ReturnsNullHandleForMissingResourceId) {
    ResourceManager manager;
    auto missing = manager.GetResource<TextResource>({});
    EXPECT_FALSE(missing.IsValid());
    EXPECT_FALSE(manager.HasResource({}));
}

TEST(ResourceManager, RejectsLoadWithoutRegisteredLoader) {
    ResourceManager manager;
    EXPECT_THROW(manager.LoadResource<UnregisteredResource>("unregistered"), common::Exception);
}

TEST(ResourceManager, RejectsCreationWithoutRegisteredFactory) {
    ResourceManager manager;
    EXPECT_THROW(manager.CreateDeferredResource<UnregisteredResource>(42), common::Exception);
}

TEST(ResourceManager, KeepsCreatedResourcesAfterLastHandleIsReleased) {
    ResourceManager manager;
    ResourceId id;
    {
        auto text = manager.CreateResource(TextResource("retained"));
        id = text.id;
    }
    auto retrieved = manager.GetResource<TextResource>(id);
    EXPECT_EQ(retrieved.GetResource()->GetText(), "retained");
}

TEST(ResourceManager, DeduplicatesNormalizedPaths) {
    ResourceManager manager;
    const auto root = std::filesystem::path(FERRUM_TEST_ASSET_ROOT);
    auto first = manager.LoadResource<TextResource>(root / "text.txt");
    auto second = manager.LoadResource<TextResource>(root / "." / "text.txt");
    EXPECT_EQ(first.id, second.id);
    EXPECT_EQ(first.handle.GetResource(), second.handle.GetResource());
    EXPECT_EQ(first.handle.GetResource()->GetText(), "Ferrum test resource\n");
}

TEST(ResourceManager, ReloadsFileAfterAllHandlesAreReleased) {
    ResourceManager manager;
    ResourceId id;
    {
        auto text = manager.LoadResource<TextResource>(
            std::filesystem::path(FERRUM_TEST_ASSET_ROOT) / "text.txt");
        id = text.id;
        EXPECT_EQ(text.handle.GetResource()->GetText(), "Ferrum test resource\n");
    }
    EXPECT_TRUE(manager.HasResource(id));
    auto reloaded = manager.GetResource<TextResource>(id);
    EXPECT_EQ(reloaded.GetResource()->GetText(), "Ferrum test resource\n");
}

TEST(ResourceManager, DefersMissingFileErrorsUntilResourceIsRequested) {
    ResourceManager manager;
    auto missing = manager.LoadResource<TextResource>(
        std::filesystem::path(FERRUM_TEST_ASSET_ROOT) / "missing.txt");
    EXPECT_TRUE(missing.handle.IsValid());
    EXPECT_THROW(missing.handle.GetResource(), common::Exception);
}

TEST(ResourceManager, RejectsResourceTypeMismatches) {
    ResourceManager manager;
    auto text = manager.CreateResource(TextResource("hello"));
    EXPECT_THROW(manager.GetResource<types::ImageResource>(text.id), common::Exception);
}

TEST(ResourceManager, ValidatesDeferredResourceTypesOnFirstAccess) {
    testing::TestGraphicsDevice device;
    ResourceManager manager;
    auto source = manager.CreateResource(TextResource("source"));
    auto shader = manager.CreateDeferredResource<renderer::ShaderModule>({
        .device = device,
        .type = renderer::ShaderType::Vertex,
        .source = source.handle,
    });
    auto mismatched = manager.GetResource<TextResource>(shader.id);
    EXPECT_EQ(device.compilation_count, 0);
    EXPECT_THROW(mismatched.GetResource(), common::Exception);
    EXPECT_NE(shader.handle.GetResource(), nullptr);
    EXPECT_EQ(device.compilation_count, 1);
}

TEST(ResourceManager, AdoptsAlreadyCreatedGpuResource) {
    testing::TestGraphicsDevice device;
    {
        ResourceManager manager;
        auto module = device.CreateShaderModule({
            .type = renderer::ShaderType::Vertex,
            .source = "source",
        });
        auto* expected = module.get();
        auto shader = manager.CreateResource(std::move(module));
        EXPECT_EQ(module, nullptr);
        EXPECT_EQ(shader.handle.GetResource(), expected);
        auto retrieved = manager.GetResource<renderer::ShaderModule>(shader.id);
        EXPECT_EQ(retrieved.GetResource(), expected);
        EXPECT_EQ(device.compilation_count, 1);
        EXPECT_EQ(device.destruction_count, 0);
    }
    EXPECT_EQ(device.destruction_count, 1);
}

TEST(ResourceManager, RejectsNullOwnedResource) {
    ResourceManager manager;
    EXPECT_THROW(manager.CreateResource(std::unique_ptr<renderer::ShaderModule>{}),
                 common::Exception);
}

}  // namespace
}  // namespace ferrum::resources

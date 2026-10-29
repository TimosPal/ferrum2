#include <gtest/gtest.h>

#include <filesystem>

#include "ferrum/common/exception.hpp"
#include "ferrum/resources/ResourceManager.hpp"

namespace ferrum::resources {
namespace {

using types::TextResource;

TEST(ResourceManager, RetrievesCreatedResourcesById) {
    ResourceManager manager;
    auto text = manager.CreateResource(TextResource("hello"));
    EXPECT_TRUE(manager.HasResource(text.id));
    auto retrieved = manager.GetResource<TextResource>(text.id);
    EXPECT_EQ(retrieved.GetResource(), text.handle.GetResource());
    EXPECT_EQ(retrieved.GetResource()->GetText(), "hello");
    EXPECT_FALSE(manager.HasResource({}));
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

}  // namespace
}  // namespace ferrum::resources

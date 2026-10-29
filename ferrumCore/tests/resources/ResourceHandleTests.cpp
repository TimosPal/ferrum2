#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <utility>

#include "ferrum/resources/ResourceHandle.hpp"
#include "ferrum/resources/types/TextResource.hpp"

namespace ferrum::resources {
namespace {

using types::TextResource;

TEST(ResourceHandle, NullHandlesCanBeCopiedAndMoved) {
    ResourceHandle<TextResource> empty(nullptr);
    auto copy = empty;
    auto moved = std::move(copy);
    EXPECT_FALSE(empty.IsValid());
    EXPECT_FALSE(copy.IsValid());
    EXPECT_FALSE(moved.IsValid());
}

TEST(ResourceHandle, CopyAndMoveConstructionMaintainReferenceCounts) {
    ResourceEntry entry{.resource = std::make_unique<TextResource>("hello")};
    {
        ResourceHandle<TextResource> first(&entry);
        EXPECT_EQ(entry.active_references, 1);
        {
            auto copy = first;
            EXPECT_EQ(entry.active_references, 2);
            auto moved = std::move(copy);
            EXPECT_FALSE(copy.IsValid());
            EXPECT_EQ(entry.active_references, 2);
            EXPECT_EQ(moved.GetResource(), first.GetResource());
        }
        EXPECT_EQ(entry.active_references, 1);
        EXPECT_TRUE(entry.HasResource());
    }
    EXPECT_EQ(entry.active_references, 0);
    EXPECT_FALSE(entry.HasResource());
}

TEST(ResourceHandle, AssignmentReleasesPreviousEntry) {
    ResourceEntry firstEntry{.resource = std::make_unique<TextResource>("first")};
    ResourceEntry secondEntry{.resource = std::make_unique<TextResource>("second")};
    ResourceHandle<TextResource> first(&firstEntry);
    ResourceHandle<TextResource> second(&secondEntry);
    second = first;
    EXPECT_EQ(secondEntry.active_references, 0);
    EXPECT_FALSE(secondEntry.HasResource());
    EXPECT_EQ(firstEntry.active_references, 2);
    EXPECT_EQ(second.GetResource(), first.GetResource());

    ResourceHandle<TextResource> destination(nullptr);
    destination = std::move(second);
    EXPECT_FALSE(second.IsValid());
    EXPECT_EQ(firstEntry.active_references, 2);
    EXPECT_EQ(destination.GetResource(), first.GetResource());
}

TEST(ResourceHandle, LoadsOnceAndEvictsAfterLastReference) {
    int loads = 0;
    ResourceEntry entry{
        .loader =
            Loader("virtual.txt", [&](const std::filesystem::path&) -> std::unique_ptr<Resource> {
                ++loads;
                return std::make_unique<TextResource>("loaded");
            })};
    {
        ResourceHandle<TextResource> first(&entry);
        auto second = first;
        EXPECT_EQ(loads, 0);
        EXPECT_EQ(first.GetResource()->GetText(), "loaded");
        EXPECT_EQ(first.GetResource(), second.GetResource());
        EXPECT_EQ(loads, 1);
    }
    EXPECT_FALSE(entry.HasResource());
    ResourceHandle<TextResource> reloaded(&entry);
    EXPECT_EQ(reloaded.GetResource()->GetText(), "loaded");
    EXPECT_EQ(loads, 2);
}

TEST(ResourceHandle, RejectsNullLoaderResults) {
    ResourceEntry entry{
        .loader = Loader(
            "virtual.txt",
            [](const std::filesystem::path&) -> std::unique_ptr<Resource> { return nullptr; })};
    ResourceHandle<TextResource> handle(&entry);
    EXPECT_THROW(handle.GetResource(), common::Exception);
}

}  // namespace
}  // namespace ferrum::resources

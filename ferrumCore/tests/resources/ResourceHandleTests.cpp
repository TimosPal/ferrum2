#include <gtest/gtest.h>

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

TEST(ResourceHandle, RejectsAccessThroughNullHandle) {
    ResourceHandle<TextResource> empty(nullptr);
    EXPECT_THROW(empty.GetResource(), common::Exception);
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
    ResourceEntry first_entry{.resource = std::make_unique<TextResource>("first")};
    ResourceEntry second_entry{.resource = std::make_unique<TextResource>("second")};
    ResourceHandle<TextResource> first(&first_entry);
    ResourceHandle<TextResource> second(&second_entry);
    second = first;
    EXPECT_EQ(second_entry.active_references, 0);
    EXPECT_FALSE(second_entry.HasResource());
    EXPECT_EQ(first_entry.active_references, 2);
    EXPECT_EQ(second.GetResource(), first.GetResource());

    ResourceHandle<TextResource> destination(nullptr);
    destination = std::move(second);
    EXPECT_FALSE(second.IsValid());
    EXPECT_EQ(first_entry.active_references, 2);
    EXPECT_EQ(destination.GetResource(), first.GetResource());
}

TEST(ResourceHandle, LoadsOnceAndEvictsAfterLastReference) {
    int loads = 0;
    ResourceEntry entry{.create = [&]() -> std::unique_ptr<Resource> {
        ++loads;
        return std::make_unique<TextResource>("loaded");
    }};
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
    ResourceEntry entry{.create = []() -> std::unique_ptr<Resource> { return nullptr; }};
    ResourceHandle<TextResource> handle(&entry);
    EXPECT_THROW(handle.GetResource(), common::Exception);
}

TEST(ResourceHandle, RejectsMissingCreationCallback) {
    ResourceEntry entry;
    ResourceHandle<TextResource> handle(&entry);
    EXPECT_THROW(handle.GetResource(), common::Exception);
}

TEST(ResourceHandle, SupportsPathlessFactoriesAndReloadsAfterEviction) {
    int calls = 0;
    ResourceEntry entry{
        .create = [&]() -> std::unique_ptr<Resource> {
            ++calls;
            return std::make_unique<TextResource>("factory");
        },
    };
    {
        ResourceHandle<TextResource> first(&entry);
        EXPECT_EQ(calls, 0);
        EXPECT_EQ(first.GetResource()->GetText(), "factory");
        EXPECT_EQ(calls, 1);
    }
    EXPECT_FALSE(entry.HasResource());
    ResourceHandle<TextResource> reloaded(&entry);
    EXPECT_EQ(reloaded.GetResource()->GetText(), "factory");
    EXPECT_EQ(calls, 2);
}

TEST(ResourceHandle, ReleasesKeptResourceCreationCapturesAfterSuccess) {
    ResourceEntry source{.resource = std::make_unique<TextResource>("source")};
    int calls = 0;
    ResourceEntry entry{
        .create = [source = ResourceHandle<TextResource>(&source),
                   &calls]() mutable -> std::unique_ptr<Resource> {
            ++calls;
            return std::make_unique<TextResource>(source.GetResource()->GetText());
        },
        .policy = ResourceEntry::EvictionPolicy::Keep,
    };
    EXPECT_EQ(source.active_references, 1);
    {
        ResourceHandle<TextResource> handle(&entry);
        auto* resource = handle.GetResource();
        EXPECT_EQ(resource->GetText(), "source");
        EXPECT_FALSE(entry.create);
        EXPECT_EQ(source.active_references, 0);
        EXPECT_FALSE(source.HasResource());
        EXPECT_EQ(handle.GetResource(), resource);
    }
    ResourceHandle<TextResource> retrieved(&entry);
    EXPECT_EQ(retrieved.GetResource()->GetText(), "source");
    EXPECT_EQ(calls, 1);
}

TEST(ResourceHandle, RetainsKeptResourceCreationCapturesAfterExceptionUntilRetrySucceeds) {
    ResourceEntry source{.resource = std::make_unique<TextResource>("source")};
    bool fail = true;
    ResourceEntry entry{
        .create = [source = ResourceHandle<TextResource>(&source),
                   &fail]() mutable -> std::unique_ptr<Resource> {
            FE_THROW_IF(fail, "Test creation failed");
            return std::make_unique<TextResource>(source.GetResource()->GetText());
        },
        .policy = ResourceEntry::EvictionPolicy::Keep,
    };
    ResourceHandle<TextResource> handle(&entry);
    EXPECT_THROW(handle.GetResource(), common::Exception);
    EXPECT_FALSE(entry.HasResource());
    EXPECT_TRUE(entry.create);
    EXPECT_EQ(source.active_references, 1);
    EXPECT_TRUE(source.HasResource());

    fail = false;
    EXPECT_EQ(handle.GetResource()->GetText(), "source");
    EXPECT_FALSE(entry.create);
    EXPECT_EQ(source.active_references, 0);
    EXPECT_FALSE(source.HasResource());
}

TEST(ResourceHandle, RetainsKeptResourceCreationCapturesAfterNullResultUntilRetrySucceeds) {
    ResourceEntry source{.resource = std::make_unique<TextResource>("source")};
    bool return_null = true;
    ResourceEntry entry{
        .create = [source = ResourceHandle<TextResource>(&source),
                   &return_null]() mutable -> std::unique_ptr<Resource> {
            if (return_null) {
                return nullptr;
            }
            return std::make_unique<TextResource>(source.GetResource()->GetText());
        },
        .policy = ResourceEntry::EvictionPolicy::Keep,
    };
    ResourceHandle<TextResource> handle(&entry);
    EXPECT_THROW(handle.GetResource(), common::Exception);
    EXPECT_FALSE(entry.HasResource());
    EXPECT_TRUE(entry.create);
    EXPECT_EQ(source.active_references, 1);
    EXPECT_TRUE(source.HasResource());

    return_null = false;
    EXPECT_EQ(handle.GetResource()->GetText(), "source");
    EXPECT_FALSE(entry.create);
    EXPECT_EQ(source.active_references, 0);
    EXPECT_FALSE(source.HasResource());
}

TEST(ResourceHandle, RetainsDiscardedResourceCreationCapturesForReloading) {
    ResourceEntry source{.resource = std::make_unique<TextResource>("source")};
    int calls = 0;
    ResourceEntry entry{
        .create = [source = ResourceHandle<TextResource>(&source),
                   &calls]() mutable -> std::unique_ptr<Resource> {
            ++calls;
            return std::make_unique<TextResource>(source.GetResource()->GetText());
        },
    };
    {
        ResourceHandle<TextResource> handle(&entry);
        EXPECT_EQ(handle.GetResource()->GetText(), "source");
        EXPECT_TRUE(entry.create);
        EXPECT_EQ(source.active_references, 1);
    }
    EXPECT_FALSE(entry.HasResource());
    EXPECT_TRUE(entry.create);
    EXPECT_EQ(source.active_references, 1);
    EXPECT_TRUE(source.HasResource());
    ResourceHandle<TextResource> reloaded(&entry);
    EXPECT_EQ(reloaded.GetResource()->GetText(), "source");
    EXPECT_EQ(calls, 2);
}

}  // namespace
}  // namespace ferrum::resources

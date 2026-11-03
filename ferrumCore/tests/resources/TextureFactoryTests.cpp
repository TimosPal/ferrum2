#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "TestGraphicsDevice.hpp"
#include "ferrum/common/exception.hpp"
#include "ferrum/resources/ResourceManager.hpp"

namespace ferrum::resources {
namespace {

using renderer::Texture;
using testing::TestGraphicsDevice;
using types::ImageResource;

static_assert(std::is_base_of_v<Resource, Texture>);
static_assert(
    std::is_same_v<decltype(std::declval<ResourceHandle<Texture>&>().GetResource()), Texture*>);

class TextureFactoryTest : public ::testing::Test {
protected:
    auto CreateTexture(renderer::TextureOptions options = {}) {
        return manager.CreateDeferredResource<Texture>({
            .device = device,
            .image = image.handle,
            .options = options,
        });
    }

    TestGraphicsDevice device;
    ResourceManager manager;
    ManagedResource<ImageResource> image =
        manager.CreateResource(ImageResource(2, 1, 3, {1, 2, 3, 4, 5, 6}));
};

TEST_F(TextureFactoryTest, CreatesAndUploadsOnlyOnFirstGetResource) {
    auto texture = CreateTexture();
    auto retrieved = manager.GetResource<Texture>(texture.id);
    EXPECT_EQ(device.texture.creation_count, 0);
    EXPECT_EQ(device.texture.upload_count, 0);

    auto* resource = texture.handle.GetResource();
    EXPECT_EQ(device.texture.creation_count, 1);
    EXPECT_EQ(device.texture.upload_count, 1);
    EXPECT_EQ(device.texture.description.width, 2);
    EXPECT_EQ(device.texture.description.height, 1);
    EXPECT_EQ(device.texture.description.channels, 3);
    EXPECT_EQ(device.texture.description.data_type, types::ImageDataType::UnsignedByte);
    EXPECT_EQ(device.texture.pixels, (std::vector<std::uint8_t>{1, 2, 3, 4, 5, 6}));
    EXPECT_EQ(device.texture.options.min_filter, renderer::TextureFilter::Linear);
    EXPECT_EQ(device.texture.options.mag_filter, renderer::TextureFilter::Linear);
    EXPECT_EQ(device.texture.options.wrap_s, renderer::TextureWrap::ClampToEdge);
    EXPECT_EQ(device.texture.options.wrap_t, renderer::TextureWrap::ClampToEdge);
    EXPECT_EQ(device.texture.options.mipmaps, renderer::MipmapMode::None);
    EXPECT_EQ(retrieved.GetResource(), resource);
    EXPECT_EQ(resource->Width(), 2);
    EXPECT_EQ(resource->Height(), 1);
    EXPECT_EQ(device.texture.creation_count, 1);
    EXPECT_EQ(device.texture.upload_count, 1);
}

TEST_F(TextureFactoryTest, PassesTextureOptionsToTheDevice) {
    auto texture = CreateTexture({
        .min_filter = renderer::TextureFilter::Nearest,
        .mag_filter = renderer::TextureFilter::Nearest,
        .wrap_s = renderer::TextureWrap::Repeat,
        .wrap_t = renderer::TextureWrap::MirroredRepeat,
        .mipmaps = renderer::MipmapMode::Linear,
    });
    texture.handle.GetResource();
    EXPECT_EQ(device.texture.options.min_filter, renderer::TextureFilter::Nearest);
    EXPECT_EQ(device.texture.options.mag_filter, renderer::TextureFilter::Nearest);
    EXPECT_EQ(device.texture.options.wrap_s, renderer::TextureWrap::Repeat);
    EXPECT_EQ(device.texture.options.wrap_t, renderer::TextureWrap::MirroredRepeat);
    EXPECT_EQ(device.texture.options.mipmaps, renderer::MipmapMode::Linear);
}

TEST_F(TextureFactoryTest, ReportsInvalidImageHandleOnGetResource) {
    auto texture = manager.CreateDeferredResource<Texture>({
        .device = device,
        .image = ResourceHandle<ImageResource>(nullptr),
    });
    EXPECT_THROW(texture.handle.GetResource(), common::Exception);
    EXPECT_EQ(device.texture.creation_count, 0);
    EXPECT_EQ(device.texture.upload_count, 0);
}

TEST_F(TextureFactoryTest, DefersImageLoadErrorsUntilGetResource) {
    auto missing = manager.LoadResource<ImageResource>(
        std::filesystem::path(FERRUM_TEST_ASSET_ROOT) / "missing.png");
    auto texture = manager.CreateDeferredResource<Texture>({
        .device = device,
        .image = missing.handle,
    });
    EXPECT_THROW(texture.handle.GetResource(), common::Exception);
    EXPECT_EQ(device.texture.creation_count, 0);
    EXPECT_EQ(device.texture.upload_count, 0);
}

TEST_F(TextureFactoryTest, PropagatesCreationErrorsAndAllowsRetry) {
    auto texture = CreateTexture();
    device.texture.fail_creation = true;
    EXPECT_THROW(texture.handle.GetResource(), common::Exception);
    EXPECT_EQ(device.texture.creation_count, 1);
    EXPECT_EQ(device.texture.upload_count, 0);
    device.texture.fail_creation = false;
    EXPECT_NO_THROW(texture.handle.GetResource());
    EXPECT_EQ(device.texture.creation_count, 2);
    EXPECT_EQ(device.texture.upload_count, 1);
}

TEST_F(TextureFactoryTest, RejectsNullTextureAndAllowsRetry) {
    auto texture = CreateTexture();
    device.texture.return_null = true;
    EXPECT_THROW(texture.handle.GetResource(), common::Exception);
    EXPECT_EQ(device.texture.creation_count, 1);
    EXPECT_EQ(device.texture.upload_count, 0);
    device.texture.return_null = false;
    EXPECT_NO_THROW(texture.handle.GetResource());
    EXPECT_EQ(device.texture.creation_count, 2);
    EXPECT_EQ(device.texture.upload_count, 1);
}

TEST_F(TextureFactoryTest, CleansUpFailedUploadAndAllowsRetry) {
    auto texture = CreateTexture();
    device.texture.fail_upload = true;
    EXPECT_THROW(texture.handle.GetResource(), common::Exception);
    EXPECT_EQ(device.texture.creation_count, 1);
    EXPECT_EQ(device.texture.upload_count, 1);
    EXPECT_EQ(device.texture.destruction_count, 1);
    device.texture.fail_upload = false;
    EXPECT_NO_THROW(texture.handle.GetResource());
    EXPECT_EQ(device.texture.creation_count, 2);
    EXPECT_EQ(device.texture.upload_count, 2);
    EXPECT_EQ(device.texture.destruction_count, 1);
}

TEST(TextureFactory, LoadsAndReleasesImageDependencyOnGetResource) {
    TestGraphicsDevice device;
    int loads = 0;
    ResourceEntry image{
        .create = [&]() -> std::unique_ptr<Resource> {
            ++loads;
            return std::make_unique<ImageResource>(1, 1, 4, std::vector<std::uint8_t>{1, 2, 3, 4});
        },
    };
    ResourceManager manager;
    auto texture = manager.CreateDeferredResource<Texture>({
        .device = device,
        .image = ResourceHandle<ImageResource>(&image),
    });
    EXPECT_EQ(loads, 0);
    EXPECT_EQ(image.active_references, 1);
    EXPECT_EQ(device.texture.creation_count, 0);

    texture.handle.GetResource();
    EXPECT_EQ(loads, 1);
    EXPECT_EQ(device.texture.creation_count, 1);
    EXPECT_EQ(device.texture.upload_count, 1);
    EXPECT_EQ(device.texture.pixels, (std::vector<std::uint8_t>{1, 2, 3, 4}));
    EXPECT_EQ(image.active_references, 0);
    EXPECT_FALSE(image.HasResource());
}

TEST(TextureFactory, ResourceManagerRetainsTextureUntilDestruction) {
    TestGraphicsDevice device;
    {
        ResourceManager manager;
        ResourceId id;
        renderer::Texture* expected = nullptr;
        {
            auto image = manager.CreateResource(ImageResource(1, 1, 4, {1, 2, 3, 4}));
            auto texture = manager.CreateDeferredResource<Texture>({
                .device = device,
                .image = image.handle,
            });
            id = texture.id;
            expected = texture.handle.GetResource();
        }
        EXPECT_EQ(device.texture.destruction_count, 0);
        auto retrieved = manager.GetResource<Texture>(id);
        EXPECT_EQ(retrieved.GetResource(), expected);
        EXPECT_EQ(device.texture.creation_count, 1);
    }
    EXPECT_EQ(device.texture.destruction_count, 1);
}

TEST(TextureFactory, DestroyingUnrequestedFactoryDoesNotCreateTexture) {
    TestGraphicsDevice device;
    {
        ResourceManager manager;
        auto image = manager.CreateResource(ImageResource(1, 1, 4, {1, 2, 3, 4}));
        auto texture = manager.CreateDeferredResource<Texture>({
            .device = device,
            .image = image.handle,
        });
    }
    EXPECT_EQ(device.texture.creation_count, 0);
    EXPECT_EQ(device.texture.upload_count, 0);
    EXPECT_EQ(device.texture.destruction_count, 0);
}

}  // namespace
}  // namespace ferrum::resources

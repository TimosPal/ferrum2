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

class FakeFontBackend final : public types::FontResource::Backend {
public:
    bool Supports(types::GlyphCodepoint codepoint) override {
        return codepoint == U'A' || codepoint == U'\u754C' || codepoint == U'\U0001F600';
    }

    types::GlyphData LoadGlyph(types::GlyphCodepoint, int pixel_size) override {
        ++glyph_load_count;
        return {.advance_x = pixel_size};
    }

    types::FontLineMetrics GetLineMetrics(int pixel_size) override {
        ++metrics_load_count;
        return {.ascent = pixel_size, .descent = -pixel_size / 4};
    }

    int glyph_load_count = 0;
    int metrics_load_count = 0;
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

TEST(ResourceManager, LoadsFontResourceThroughGenericLoader) {
    ResourceManager manager;
    const auto missing_font = std::filesystem::path(FERRUM_TEST_ASSET_ROOT) / "missing.ttf";
    auto font = manager.LoadResource<types::FontResource>(missing_font);
    auto same_font = manager.LoadResource<types::FontResource>(missing_font.parent_path() / "." /
                                                               missing_font.filename());

    EXPECT_EQ(font.id, same_font.id);
    EXPECT_THROW(font.handle.GetResource(), common::Exception);
}

TEST(FontResource, LazilyLoadsAndCachesGlyphsByCodepointAndSize) {
    auto backend = std::make_unique<FakeFontBackend>();
    auto* backend_ptr = backend.get();
    types::FontResource font(std::move(backend));

    EXPECT_TRUE(font.Supports(U'A'));
    EXPECT_TRUE(font.Supports(U'\u754C'));
    EXPECT_TRUE(font.Supports(U'\U0001F600'));
    EXPECT_FALSE(font.Supports(U'B'));

    const auto& small = font.GetGlyph(U'\u754C', 12);
    const auto& same_small = font.GetGlyph(U'\u754C', 12);
    const auto& large = font.GetGlyph(U'\u754C', 24);
    EXPECT_EQ(small.advance_x, 12);
    EXPECT_EQ(large.advance_x, 24);
    EXPECT_EQ(&small, &same_small);
    EXPECT_EQ(backend_ptr->glyph_load_count, 2);

    EXPECT_EQ(font.GetLineMetrics(12).ascent, 12);
    EXPECT_EQ(font.GetLineMetrics(12).ascent, 12);
    EXPECT_EQ(font.GetLineMetrics(24).ascent, 24);
    EXPECT_EQ(backend_ptr->metrics_load_count, 2);

    EXPECT_THROW(font.GetGlyph(U'B', 12), common::Exception);
    EXPECT_THROW(font.GetGlyph(U'A', 0), common::Exception);
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

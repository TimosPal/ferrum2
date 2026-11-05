#include "ferrum/resources/types/FontResource.hpp"

#include <cstddef>
#include <limits>
#include <utility>

#include "ferrum/common/exception.hpp"

namespace ferrum::resources::types {
namespace {

void ValidateGlyphData(const GlyphData& glyph) {
    FE_THROW_IF(glyph.bitmap_width < 0 || glyph.bitmap_height < 0 ||
                    ((glyph.bitmap_width == 0) != (glyph.bitmap_height == 0)),
                "Font glyph has invalid bitmap dimensions");
    if (!glyph.IsVisible()) {
        FE_THROW_IF(!glyph.bitmap.empty(), "Invisible font glyph has bitmap data");
        return;
    }

    const auto width = static_cast<std::size_t>(glyph.bitmap_width);
    const auto height = static_cast<std::size_t>(glyph.bitmap_height);
    FE_THROW_IF(width > std::numeric_limits<std::size_t>::max() / height,
                "Font glyph bitmap is too large");
    FE_THROW_IF(glyph.bitmap.size() != width * height,
                "Font glyph bitmap data does not match its dimensions");
}

}  // namespace

FontResource::FontResource(std::unique_ptr<Backend> backend) : backend_(std::move(backend)) {
    FE_THROW_IF(backend_ == nullptr, "Font resource requires a font backend");
}

bool FontResource::Supports(GlyphCodepoint codepoint) const {
    if (!IsUnicodeScalarValue(codepoint)) {
        return false;
    }
    return backend_->Supports(codepoint);
}

const GlyphData& FontResource::GetGlyph(GlyphCodepoint codepoint, int pixel_size) const {
    FE_THROW_IF(!IsUnicodeScalarValue(codepoint), "Invalid Unicode code point {}",
                static_cast<std::uint32_t>(codepoint));
    FE_THROW_IF(pixel_size <= 0, "Font pixel size must be positive");

    const GlyphKey key{codepoint, pixel_size};
    auto glyph = glyphs_.find(key);
    if (glyph != glyphs_.end()) {
        return glyph->second;
    }
    FE_THROW_IF(!backend_->Supports(codepoint), "Font does not contain glyph {}",
                static_cast<std::uint32_t>(codepoint));

    auto loaded_glyph = backend_->LoadGlyph(codepoint, pixel_size);
    ValidateGlyphData(loaded_glyph);
    return glyphs_.emplace(key, std::move(loaded_glyph)).first->second;
}

FontLineMetrics FontResource::GetLineMetrics(int pixel_size) const {
    FE_THROW_IF(pixel_size <= 0, "Font pixel size must be positive");

    const auto metrics = line_metrics_.find(pixel_size);
    if (metrics != line_metrics_.end()) {
        return metrics->second;
    }
    return line_metrics_.emplace(pixel_size, backend_->GetLineMetrics(pixel_size)).first->second;
}

}  // namespace ferrum::resources::types

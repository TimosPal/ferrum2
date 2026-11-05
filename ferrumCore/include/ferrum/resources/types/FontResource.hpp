#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "ferrum/resources/Resource.hpp"

namespace ferrum::resources::types {

using GlyphCodepoint = char32_t;

// Reject values that are not valid Unicode characters.
constexpr bool IsUnicodeScalarValue(GlyphCodepoint codepoint) noexcept {
    return codepoint <= 0x10FFFF && (codepoint < 0xD800 || codepoint > 0xDFFF);
}

struct GlyphData {
    int advance_x = 0;
    int bitmap_width = 0;
    int bitmap_height = 0;
    int bearing_x = 0;
    int bearing_y = 0;
    std::vector<std::uint8_t> bitmap;

    bool IsVisible() const noexcept { return bitmap_width > 0 && bitmap_height > 0; }
};

struct FontLineMetrics {
    int ascent = 0;
    int descent = 0;
};

class FontResource final : public Resource {
public:
    class Backend {
    public:
        virtual ~Backend() = default;
        virtual bool Supports(GlyphCodepoint codepoint) = 0;
        virtual GlyphData LoadGlyph(GlyphCodepoint codepoint, int pixel_size) = 0;
        virtual FontLineMetrics GetLineMetrics(int pixel_size) = 0;
    };

    explicit FontResource(std::unique_ptr<Backend> backend);

    bool Supports(GlyphCodepoint codepoint) const;
    const GlyphData& GetGlyph(GlyphCodepoint codepoint, int pixel_size) const;
    FontLineMetrics GetLineMetrics(int pixel_size) const;

private:
    // The pixel size is part of the key because each size has a different glyph bitmap.
    using GlyphKey = std::pair<GlyphCodepoint, int>;

    std::unique_ptr<Backend> backend_;
    mutable std::map<GlyphKey, GlyphData> glyphs_;
    // Line metrics also differ by pixel size.
    mutable std::map<int, FontLineMetrics> line_metrics_;
};

}  // namespace ferrum::resources::types

#include "ferrum/resources/FontBackend.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <limits>
#include <string>

#include "ferrum/common/exception.hpp"

namespace ferrum::resources::loaders::detail {
namespace {

struct FreeTypeLibrary {
    FT_Library handle = nullptr;

    ~FreeTypeLibrary() {
        if (handle != nullptr) {
            FT_Done_FreeType(handle);
        }
    }
};

struct FreeTypeFace {
    FT_Face handle = nullptr;

    ~FreeTypeFace() {
        if (handle != nullptr) {
            FT_Done_Face(handle);
        }
    }
};

void SetPixelSize(FT_Face face, int pixel_size) {
    FE_THROW_IF(pixel_size <= 0, "Font pixel size must be positive");
    const auto error = FT_Set_Pixel_Sizes(face, 0, static_cast<FT_UInt>(pixel_size));
    FE_THROW_IF(error != 0, "Failed to set font pixel size to {} (FreeType error {})", pixel_size,
                error);
}

types::GlyphData RasterizeGlyph(FT_Face face, types::GlyphCodepoint codepoint) {
    const auto freetype_codepoint = static_cast<FT_ULong>(codepoint);
    const auto glyph_index = FT_Get_Char_Index(face, freetype_codepoint);
    FE_THROW_IF(glyph_index == 0, "Font does not contain glyph {}",
                static_cast<std::uint32_t>(freetype_codepoint));

    const auto error = FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER);
    FE_THROW_IF(error != 0, "Failed to load font glyph {}",
                static_cast<std::uint32_t>(freetype_codepoint));

    const auto& slot = *face->glyph;
    const auto& bitmap = slot.bitmap;
    FE_THROW_IF(bitmap.width > static_cast<unsigned int>(std::numeric_limits<int>::max()) ||
                    bitmap.rows > static_cast<unsigned int>(std::numeric_limits<int>::max()),
                "Font glyph dimensions exceed the supported range");

    types::GlyphData glyph{
        .advance_x = static_cast<int>(slot.advance.x >> 6),
        .bitmap_width = static_cast<int>(bitmap.width),
        .bitmap_height = static_cast<int>(bitmap.rows),
        .bearing_x = slot.bitmap_left,
        .bearing_y = slot.bitmap_top,
    };
    if (!glyph.IsVisible()) {
        return glyph;
    }

    const auto width = static_cast<std::size_t>(glyph.bitmap_width);
    const auto height = static_cast<std::size_t>(glyph.bitmap_height);
    FE_THROW_IF(width > std::numeric_limits<std::size_t>::max() / height,
                "Font glyph bitmap is too large");
    glyph.bitmap.resize(width * height);

    const auto minimum_pitch = bitmap.pixel_mode == FT_PIXEL_MODE_MONO ? (width + 7) / 8 : width;
    const auto pitch = static_cast<std::size_t>(std::abs(static_cast<long long>(bitmap.pitch)));
    FE_THROW_IF(
        (bitmap.pixel_mode != FT_PIXEL_MODE_GRAY && bitmap.pixel_mode != FT_PIXEL_MODE_MONO) ||
            pitch < minimum_pitch,
        "Unsupported or invalid FreeType glyph bitmap format");
    FE_THROW_IF(bitmap.pixel_mode == FT_PIXEL_MODE_GRAY && bitmap.num_grays < 2,
                "FreeType returned an invalid grayscale range for a glyph");

    for (std::size_t row = 0; row < height; ++row) {
        const auto* source_row = bitmap.buffer + (static_cast<std::ptrdiff_t>(row) * bitmap.pitch);
        auto* destination_row = glyph.bitmap.data() + (row * width);
        if (bitmap.pixel_mode == FT_PIXEL_MODE_GRAY) {
            if (bitmap.num_grays == 256) {
                std::copy_n(source_row, width, destination_row);
                continue;
            }
            const auto max_gray = static_cast<unsigned int>(bitmap.num_grays - 1);
            for (std::size_t column = 0; column < width; ++column) {
                const auto gray = std::min(static_cast<unsigned int>(source_row[column]), max_gray);
                destination_row[column] = static_cast<std::uint8_t>((gray * 255U) / max_gray);
            }
        } else {
            for (std::size_t column = 0; column < width; ++column) {
                const auto bit = static_cast<unsigned char>(0x80U >> (column % 8));
                destination_row[column] = (source_row[column / 8] & bit) != 0 ? 255U : 0U;
            }
        }
    }

    return glyph;
}

class FreeTypeFontBackend final : public types::FontResource::Backend {
public:
    explicit FreeTypeFontBackend(const std::filesystem::path& path) {
        FE_THROW_IF(path.empty(), "Font resource path cannot be empty");
        auto error = FT_Init_FreeType(&library_.handle);
        FE_THROW_IF(error != 0, "Failed to initialize FreeType (error {})", error);

        const auto path_string = path.string();
        error = FT_New_Face(library_.handle, path_string.c_str(), 0, &face_.handle);
        FE_THROW_IF(error != 0, "Failed to load font resource '{}' (FreeType error {})",
                    path_string, error);
    }

    bool Supports(types::GlyphCodepoint codepoint) override {
        return FT_Get_Char_Index(face_.handle, static_cast<FT_ULong>(codepoint)) != 0;
    }

    types::GlyphData LoadGlyph(types::GlyphCodepoint codepoint, int pixel_size) override {
        SetPixelSize(face_.handle, pixel_size);
        return RasterizeGlyph(face_.handle, codepoint);
    }

    types::FontLineMetrics GetLineMetrics(int pixel_size) override {
        SetPixelSize(face_.handle, pixel_size);
        const auto& metrics = face_.handle->size->metrics;
        return {
            .ascent = static_cast<int>(metrics.ascender >> 6),
            .descent = static_cast<int>(metrics.descender >> 6),
        };
    }

private:
    FreeTypeLibrary library_;
    FreeTypeFace face_;
};

}  // namespace

std::unique_ptr<types::FontResource::Backend> CreateFontBackend(const std::filesystem::path& path) {
    return std::make_unique<FreeTypeFontBackend>(path);
}

}  // namespace ferrum::resources::loaders::detail

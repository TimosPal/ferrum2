#include "ferrum/resources/loaders.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <fstream>
#include <iterator>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ferrum/common/exception.hpp"
#include "ferrum/resources/types/ImageResource.hpp"
#include "ferrum/resources/types/TextResource.hpp"

namespace ferrum::resources::loaders {

std::unique_ptr<Resource> LoadTextResource(std::filesystem::path const& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        FE_THROW("Failed to open text resource '{}'", path.string());
    }

    std::string text(std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{});
    if (file.bad()) {
        FE_THROW("Failed to read text resource '{}'", path.string());
    }
    return std::make_unique<types::TextResource>(std::move(text));
}

std::unique_ptr<Resource> LoadImageResource(std::filesystem::path const& path) {
    int width = 0;
    int height = 0;
    int channels = 0;
    const std::string path_string = path.string();
    std::unique_ptr<stbi_uc, decltype(&stbi_image_free)> image(
        stbi_load(path_string.c_str(), &width, &height, &channels, 0), &stbi_image_free);
    if (!image) {
        const char* reason = stbi_failure_reason();
        FE_THROW("Failed to load image resource '{}': {}", path_string,
                 reason != nullptr ? reason : "unknown stb_image error");
    }

    if (width <= 0 || height <= 0 || channels <= 0 || channels > 4) {
        FE_THROW("Image resource '{}' has invalid dimensions or channel count", path_string);
    }
    const auto width_size = static_cast<std::size_t>(width);
    const auto height_size = static_cast<std::size_t>(height);
    if (width_size > std::numeric_limits<std::size_t>::max() / height_size) {
        FE_THROW("Image resource '{}' is too large", path_string);
    }
    const auto pixel_count = width_size * height_size;
    if (pixel_count >
        std::numeric_limits<std::size_t>::max() / static_cast<std::size_t>(channels)) {
        FE_THROW("Image resource '{}' is too large", path_string);
    }
    const auto data_size = pixel_count * static_cast<std::size_t>(channels);
    std::vector<std::uint8_t> pixels(image.get(), image.get() + data_size);
    return std::make_unique<types::ImageResource>(width, height, channels, std::move(pixels));
}

}  // namespace ferrum::resources::loaders

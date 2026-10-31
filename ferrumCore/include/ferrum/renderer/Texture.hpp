#pragma once

#include <glm/vec2.hpp>

#include "ferrum/resources/types/ImageResource.hpp"

namespace ferrum::renderer {

enum class TextureFilter {
    Nearest,
    Linear,
};

enum class TextureWrap {
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder,
};

enum class MipmapMode {
    None,
    Nearest,
    Linear,
};

struct TextureOptions {
    TextureFilter min_filter = TextureFilter::Linear;
    TextureFilter mag_filter = TextureFilter::Linear;
    TextureWrap wrap_s = TextureWrap::ClampToEdge;
    TextureWrap wrap_t = TextureWrap::ClampToEdge;
    MipmapMode mipmaps = MipmapMode::None;
};

struct TextureDesc {
    int width{};
    int height{};
    int channels{};
    resources::types::ImageDataType data_type = resources::types::ImageDataType::UnsignedByte;
};

class Texture {
public:
    virtual ~Texture() = default;
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&&) noexcept = default;
    Texture& operator=(Texture&&) noexcept = default;

    virtual int Width() const noexcept = 0;
    virtual int Height() const noexcept = 0;
    virtual int MipmapLevels() const noexcept = 0;
    virtual void Upload(const resources::types::ImageResource& image) = 0;
    virtual void UploadRegion(const resources::types::ImageResource& image, glm::ivec2 offset) = 0;

protected:
    Texture() = default;
};

}  // namespace ferrum::renderer

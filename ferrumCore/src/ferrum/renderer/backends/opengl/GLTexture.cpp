#include "ferrum/renderer/backends/opengl/GLTexture.hpp"

#include <glad/gl.h>

#include <cstddef>
#include <limits>
#include <utility>

#include "ferrum/common/exception.hpp"
#include "ferrum/renderer/backends/opengl/utils.hpp"

namespace ferrum::renderer::opengl {
namespace {

struct OpenGLTextureFormat {
    GLenum upload_format;
    GLenum internal_format;
};

OpenGLTextureFormat ToOpenGLTextureFormat(int channels) {
    switch (channels) {
        case 1:
            return {.upload_format = GL_RED, .internal_format = GL_R8};
        case 2:
            return {.upload_format = GL_RG, .internal_format = GL_RG8};
        case 3:
            return {.upload_format = GL_RGB, .internal_format = GL_RGB8};
        case 4:
            return {.upload_format = GL_RGBA, .internal_format = GL_RGBA8};
        default:
            FE_THROW("Unsupported image channel count: {}", channels);
    }
}

void ValidateImageDataType(resources::types::ImageDataType data_type) {
    switch (data_type) {
        case resources::types::ImageDataType::UnsignedByte:
            return;
        default:
            FE_THROW("Unsupported image data type");
    }
}

GLenum ToOpenGLPixelDataType(resources::types::ImageDataType data_type) {
    ValidateImageDataType(data_type);
    return GL_UNSIGNED_BYTE;
}

GLenum ToFilter(TextureFilter filter) {
    switch (filter) {
        case TextureFilter::Nearest:
            return GL_NEAREST;
        case TextureFilter::Linear:
            return GL_LINEAR;
        default:
            FE_THROW("Unsupported texture filter");
    }
}

GLenum ToWrap(TextureWrap wrap) {
    switch (wrap) {
        case TextureWrap::Repeat:
            return GL_REPEAT;
        case TextureWrap::MirroredRepeat:
            return GL_MIRRORED_REPEAT;
        case TextureWrap::ClampToEdge:
            return GL_CLAMP_TO_EDGE;
        case TextureWrap::ClampToBorder:
            return GL_CLAMP_TO_BORDER;
        default:
            FE_THROW("Unsupported texture wrap mode");
    }
}

GLenum ToMinFilter(TextureFilter filter, MipmapMode mipmaps) {
    if (mipmaps == MipmapMode::None) {
        return ToFilter(filter);
    }
    if (filter == TextureFilter::Nearest && mipmaps == MipmapMode::Nearest) {
        return GL_NEAREST_MIPMAP_NEAREST;
    }
    if (filter == TextureFilter::Linear && mipmaps == MipmapMode::Nearest) {
        return GL_LINEAR_MIPMAP_NEAREST;
    }
    if (filter == TextureFilter::Nearest && mipmaps == MipmapMode::Linear) {
        return GL_NEAREST_MIPMAP_LINEAR;
    }
    if (filter == TextureFilter::Linear && mipmaps == MipmapMode::Linear) {
        return GL_LINEAR_MIPMAP_LINEAR;
    }
    FE_THROW("Unsupported texture mipmap mode");
}

int CalculateMipmapLevels(int width, int height, MipmapMode mipmaps) {
    if (mipmaps == MipmapMode::None) {
        return 1;
    }

    int levels = 1;
    for (int dimension = width > height ? width : height; dimension > 1; dimension /= 2) {
        ++levels;
    }
    return levels;
}

std::size_t ExpectedPixelCount(const resources::types::ImageResource& image) {
    FE_THROW_IF(image.GetWidth() <= 0 || image.GetHeight() <= 0,
                "Texture dimensions must be positive");
    FE_THROW_IF(image.GetChannels() < 1 || image.GetChannels() > 4,
                "Texture channel count must be between 1 and 4");

    const auto width = static_cast<std::size_t>(image.GetWidth());
    const auto height = static_cast<std::size_t>(image.GetHeight());
    const auto channels = static_cast<std::size_t>(image.GetChannels());
    FE_THROW_IF(width > std::numeric_limits<std::size_t>::max() / height ||
                    width * height > std::numeric_limits<std::size_t>::max() / channels,
                "Texture is too large");
    return width * height * channels;
}

void SetTextureParameters(GLuint id, TextureOptions options) {
    FE_GL(glTextureParameteri(id, GL_TEXTURE_MIN_FILTER,
                              ToMinFilter(options.min_filter, options.mipmaps)));
    FE_GL(glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, ToFilter(options.mag_filter)));
    FE_GL(glTextureParameteri(id, GL_TEXTURE_WRAP_S, ToWrap(options.wrap_s)));
    FE_GL(glTextureParameteri(id, GL_TEXTURE_WRAP_T, ToWrap(options.wrap_t)));
}

void UploadTexture(GLuint id, const resources::types::ImageResource& image, glm::ivec2 offset,
                   GLenum format, GLenum data_type) {
    GLint previous_unpack_alignment = 0;
    FE_GL(glGetIntegerv(GL_UNPACK_ALIGNMENT, &previous_unpack_alignment));
    FE_GL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    try {
        FE_GL(glTextureSubImage2D(id, 0, offset.x, offset.y, image.GetWidth(), image.GetHeight(),
                                  format, data_type, image.GetPixels().data()));
    } catch (...) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, previous_unpack_alignment);
        throw;
    }
    FE_GL(glPixelStorei(GL_UNPACK_ALIGNMENT, previous_unpack_alignment));
}

void GenerateMipmaps(GLuint id, MipmapMode mipmaps) {
    if (mipmaps != MipmapMode::None) {
        FE_GL(glGenerateTextureMipmap(id));
    }
}

unsigned int MaxTextureUnits() {
    static const unsigned int max_texture_units = [] {
        GLint value = 0;
        FE_GL(glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &value));
        FE_THROW_IF(value <= 0, "OpenGL reported no texture units");
        return static_cast<unsigned int>(value);
    }();
    return max_texture_units;
}

}  // namespace

GLTexture::GLTexture(TextureDesc description, TextureOptions options) {
    FE_GL(glCreateTextures(GL_TEXTURE_2D, 1, &id_));
    FE_THROW_IF(!IsValid(), "Failed to create OpenGL texture");
    try {
        InitializeStorage(description, options);
        SetTextureParameters(id_, options);
    } catch (...) {
        glDeleteTextures(1, &id_);
        id_ = 0;
        throw;
    }
}

void GLTexture::InitializeStorage(TextureDesc description, TextureOptions options) {
    FE_THROW_IF(description.width <= 0 || description.height <= 0,
                "Texture dimensions must be positive");
    const auto format = ToOpenGLTextureFormat(description.channels);
    ValidateImageDataType(description.data_type);
    const auto mipmap_levels =
        CalculateMipmapLevels(description.width, description.height, options.mipmaps);
    FE_GL(glTextureStorage2D(id_, mipmap_levels, format.internal_format, description.width,
                             description.height));
    storage_ = StorageInfo{
        .description = description,
        .options = options,
        .mipmap_levels = mipmap_levels,
    };
}

void GLTexture::Upload(const resources::types::ImageResource& image) {
    const auto pixel_count = ExpectedPixelCount(image);
    FE_THROW_IF(image.GetPixels().size() != pixel_count,
                "Texture pixel data does not match its dimensions");

    const auto format = ToOpenGLTextureFormat(image.GetChannels());
    FE_THROW_IF(image.GetWidth() != storage_.description.width ||
                    image.GetHeight() != storage_.description.height,
                "Texture upload dimensions do not match allocated storage");
    FE_THROW_IF(image.GetChannels() != storage_.description.channels ||
                    image.GetDataType() != storage_.description.data_type,
                "Texture upload format does not match allocated storage");

    UploadTexture(id_, image, {0, 0}, format.upload_format,
                  ToOpenGLPixelDataType(image.GetDataType()));
    GenerateMipmaps(id_, storage_.options.mipmaps);
}

void GLTexture::UploadRegion(const resources::types::ImageResource& image, glm::ivec2 offset) {
    const auto pixel_count = ExpectedPixelCount(image);
    FE_THROW_IF(image.GetPixels().size() != pixel_count,
                "Texture pixel data does not match its dimensions");
    FE_THROW_IF(image.GetChannels() != storage_.description.channels ||
                    image.GetDataType() != storage_.description.data_type,
                "Texture upload format does not match allocated storage");
    FE_THROW_IF(offset.x < 0 || offset.y < 0,
                "Texture upload offset must be non-negative and within the texture");
    FE_THROW_IF(offset.x > storage_.description.width - image.GetWidth() ||
                    offset.y > storage_.description.height - image.GetHeight(),
                "Texture upload region exceeds texture bounds");

    const auto format = ToOpenGLTextureFormat(image.GetChannels());
    UploadTexture(id_, image, offset, format.upload_format,
                  ToOpenGLPixelDataType(image.GetDataType()));
    GenerateMipmaps(id_, storage_.options.mipmaps);
}

GLTexture::~GLTexture() {
    if (id_ != 0) {
        glDeleteTextures(1, &id_);
    }
}

GLTexture::GLTexture(GLTexture&& other) noexcept
    : id_(std::exchange(other.id_, 0)), storage_(other.storage_) {
    other.storage_ = {};
}

GLTexture& GLTexture::operator=(GLTexture&& other) noexcept {
    if (this != &other) {
        if (id_ != 0) {
            glDeleteTextures(1, &id_);
        }
        id_ = std::exchange(other.id_, 0);
        storage_ = other.storage_;
        other.storage_ = {};
    }
    return *this;
}

void GLTexture::Bind(unsigned int texture_unit) const {
    detail::RequireValid(IsValid(), "bind", "OpenGL texture");
    const auto max_texture_units = MaxTextureUnits();
    FE_THROW_IF(texture_unit >= max_texture_units, "Texture unit {} exceeds OpenGL limit of {}",
                texture_unit, max_texture_units);
    FE_GL(glBindTextureUnit(texture_unit, id_));
}

}  // namespace ferrum::renderer::opengl

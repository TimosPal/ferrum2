#pragma once

#include "ferrum/renderer/Texture.hpp"

namespace ferrum::renderer::opengl {

class GLTexture final : public renderer::Texture {
public:
    explicit GLTexture(TextureDesc description, TextureOptions options = {});
    ~GLTexture() override;
    GLTexture(const GLTexture&) = delete;
    GLTexture& operator=(const GLTexture&) = delete;
    GLTexture(GLTexture&& other) noexcept;
    GLTexture& operator=(GLTexture&& other) noexcept;

    bool IsValid() const noexcept { return id_ != 0; }
    unsigned int Id() const noexcept { return id_; }
    int Width() const noexcept override { return storage_.description.width; }
    int Height() const noexcept override { return storage_.description.height; }
    int MipmapLevels() const noexcept override { return storage_.mipmap_levels; }
    void Upload(const resources::types::ImageResource& image) override;
    void UploadRegion(const resources::types::ImageResource& image, glm::ivec2 offset) override;
    void Bind(unsigned int texture_unit) const override;

private:
    void InitializeStorage(TextureDesc description, TextureOptions options);

    struct StorageInfo {
        TextureDesc description{};
        TextureOptions options{};
        int mipmap_levels = 1;
    };

    unsigned int id_ = 0;
    StorageInfo storage_;
};

}  // namespace ferrum::renderer::opengl

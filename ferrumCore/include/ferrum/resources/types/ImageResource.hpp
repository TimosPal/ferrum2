#pragma once

#include <cstdint>
#include <span>
#include <utility>
#include <vector>

#include "ferrum/resources/Resource.hpp"

namespace ferrum::resources::types {

enum class ImageDataType {
    UnsignedByte,
};

class ImageResource : public ferrum::resources::Resource {
public:
    ImageResource(int width, int height, int channels, std::vector<std::uint8_t> pixels,
                  ImageDataType data_type = ImageDataType::UnsignedByte)
        : width_(width),
          height_(height),
          channels_(channels),
          data_type_(data_type),
          pixels_(std::move(pixels)) {}

    int GetWidth() const { return width_; }
    int GetHeight() const { return height_; }
    int GetChannels() const { return channels_; }
    ImageDataType GetDataType() const noexcept { return data_type_; }
    std::span<const std::uint8_t> GetPixels() const noexcept { return pixels_; }

private:
    int width_;
    int height_;
    int channels_;
    ImageDataType data_type_;
    std::vector<std::uint8_t> pixels_;
};

}  // namespace ferrum::resources::types

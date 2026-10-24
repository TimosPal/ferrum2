#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "ferrum/resources/Resource.hpp"

namespace ferrum::resources::types {

class ImageResource : public ferrum::resources::Resource {
public:
    ImageResource(int width, int height, int channels, std::vector<std::uint8_t> pixels)
        : width_(width), height_(height), channels_(channels), pixels_(std::move(pixels)) {}

    int GetWidth() const { return width_; }
    int GetHeight() const { return height_; }
    int GetChannels() const { return channels_; }
    const std::vector<std::uint8_t>& GetPixels() const { return pixels_; }

private:
    int width_;
    int height_;
    int channels_;
    std::vector<std::uint8_t> pixels_;
};

}  // namespace ferrum::resources::types

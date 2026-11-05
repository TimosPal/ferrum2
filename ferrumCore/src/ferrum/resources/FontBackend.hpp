#pragma once

#include <filesystem>
#include <memory>

#include "ferrum/resources/types/FontResource.hpp"

namespace ferrum::resources::loaders::detail {

std::unique_ptr<types::FontResource::Backend> CreateFontBackend(const std::filesystem::path& path);

}  // namespace ferrum::resources::loaders::detail

#pragma once

#include <filesystem>
#include <memory>

#include "ferrum/resources/Resource.hpp"

namespace ferrum::resources::loaders {

std::unique_ptr<Resource> LoadTextResource(std::filesystem::path const& path);
std::unique_ptr<Resource> LoadImageResource(std::filesystem::path const& path);

}  // namespace ferrum::resources::loaders

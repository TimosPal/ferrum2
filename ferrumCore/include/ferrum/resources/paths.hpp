#pragma once

#include <filesystem>

namespace ferrum::resources {

// Development asset location configured by CMake.
inline const std::filesystem::path& CoreAssetRoot() {
    static const std::filesystem::path root(FERRUM_CORE_ASSET_ROOT);
    return root;
}

}  // namespace ferrum::resources

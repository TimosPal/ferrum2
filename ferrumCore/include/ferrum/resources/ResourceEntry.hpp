#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <utility>

#include "ferrum/common/exception.hpp"
#include "ferrum/resources/LoaderManager.hpp"
#include "ferrum/resources/Resource.hpp"

namespace ferrum::resources {

class Loader {
public:
    Loader(std::filesystem::path path, LoaderManager::LoaderCallback callback)
        : load_path(std::move(path)), loader_cb(std::move(callback)) {}
    bool HasLoadPath() const { return !load_path.empty(); }
    bool HasLoaderCallback() const { return loader_cb != nullptr; }
    bool IsValid() const { return HasLoadPath() && HasLoaderCallback(); }
    auto Load() {
        FE_THROW_IF(!IsValid(), "Loader is not valid");
        return loader_cb(load_path);
    }

private:
    std::filesystem::path load_path;
    LoaderManager::LoaderCallback loader_cb = nullptr;
};

struct ResourceEntry {
    std::optional<Loader> loader;
    std::unique_ptr<Resource> resource;
    // How the resource should be evicted when no longer referenced by any handles
    enum class EvictionPolicy {
        Keep,
        Discard,
    } policy = EvictionPolicy::Discard;

    size_t active_references = 0;

    bool HasResource() const { return resource != nullptr; }
    bool HasLoadPath() const { return loader.has_value() && loader->HasLoadPath(); }
    Resource* Load() {
        if (HasResource()) {
            // If already cached by other handle, return it directly
            return resource.get();
        }

        FE_THROW_IF(!loader.has_value(), "Loader is not set");
        // Load can throw
        resource = loader->Load();
        return resource.get();
    }
    void Reset() { resource.reset(); }
};

}  // namespace ferrum::resources

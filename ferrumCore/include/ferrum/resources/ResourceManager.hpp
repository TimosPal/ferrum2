#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <utility>

#include "ferrum/common/exception.hpp"
#include "ferrum/containers/SlotMap.hpp"
#include "ferrum/resources/LoaderManager.hpp"
#include "ferrum/resources/ResourceEntry.hpp"
#include "ferrum/resources/ResourceHandle.hpp"
#include "ferrum/resources/loaders.hpp"
#include "ferrum/resources/types/ImageResource.hpp"
#include "ferrum/resources/types/TextResource.hpp"

namespace ferrum::resources {

using ResourceId = containers::SlotMap<std::unique_ptr<ResourceEntry>>::Key;

template <typename T>
struct ManagedResource {
    ResourceId id;
    ResourceHandle<T> handle;
};

class ResourceManager {
public:
    ResourceManager() {
        loaders_.RegisterLoader<types::TextResource>(loaders::LoadTextResource);
        loaders_.RegisterLoader<types::ImageResource>(loaders::LoadImageResource);
    }
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;
    ~ResourceManager() = default;

    template <typename T>
    auto CreateResource(T&& resource);

    template <typename T>
    auto LoadResource(const std::filesystem::path& load_path);

    template <typename T>
    auto GetResource(ResourceId id);

    bool HasResource(ResourceId id) const;

private:
    containers::SlotMap<std::unique_ptr<ResourceEntry>> resources_;
    using key = ResourceId;
    using ResourceKey = std::tuple<std::filesystem::path, std::type_index>;
    std::map<ResourceKey, key> dedups_;
    LoaderManager loaders_;
};

template <typename T>
inline auto ResourceManager::GetResource(ResourceId id) {
    auto* entry_ptr = resources_.Get(id);
    bool entry_exists = entry_ptr && *entry_ptr;
    if (!entry_exists) {
        // Not found in the resource manager, return a null handle
        return ResourceHandle<T>(nullptr);
    }
    ResourceEntry& entry = **entry_ptr;
    ResourceHandle<T> handle(&entry);
    return handle;
}

inline bool ResourceManager::HasResource(ResourceId id) const {
    const auto* entry_ptr = resources_.Get(id);
    return (entry_ptr != nullptr) && *entry_ptr;
}

template <typename T>
inline auto ResourceManager::LoadResource(const std::filesystem::path& load_path) {
    // paths with different representations (e.g., relative vs absolute) should be normalized
    // so they dedup the same resource
    std::filesystem::path normalized_path = std::filesystem::weakly_canonical(load_path);
    const ResourceKey resource_key{normalized_path, std::type_index(typeid(T))};
    auto it = dedups_.find(resource_key);
    if (it != dedups_.end()) {
        return ManagedResource<T>{it->second, ResourceHandle<T>(resources_.Get(it->second)->get())};
    }

    auto loader_callback = loaders_.GetLoader<T>();
    FE_THROW_IF(loader_callback == nullptr, "Loader callback is not set");
    std::unique_ptr<ResourceEntry> entry = std::make_unique<ResourceEntry>(ResourceEntry{
        .loader = Loader{normalized_path, loader_callback},
        .policy = ResourceEntry::EvictionPolicy::Discard,
    });
    auto key = resources_.Create(std::move(entry));
    dedups_.emplace(resource_key, key);
    return ManagedResource<T>{
        key,
        ResourceHandle<T>(resources_.Get(key)->get()),
    };
}

template <typename T>
inline auto ResourceManager::CreateResource(T&& resource) {
    using ResourceType = std::remove_cvref_t<T>;
    std::unique_ptr<ResourceEntry> entry = std::make_unique<ResourceEntry>(ResourceEntry{
        .resource = std::make_unique<ResourceType>(std::forward<T>(resource)),
        .policy = ResourceEntry::EvictionPolicy::Keep,  // Created resources cant be re-loaded so
                                                        // they are kept
    });
    auto key = resources_.Create(std::move(entry));
    return ManagedResource<ResourceType>{
        key,
        ResourceHandle<ResourceType>(resources_.Get(key)->get()),
    };
}

}  // namespace ferrum::resources
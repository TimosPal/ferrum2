#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>

#include "ferrum/resources/Resource.hpp"

namespace ferrum::resources {

class LoaderManager {
public:
    using LoaderCallback = std::function<std::unique_ptr<Resource>(std::filesystem::path const&)>;

    LoaderManager() = default;
    LoaderManager(const LoaderManager&) = default;
    LoaderManager& operator=(const LoaderManager&) = default;
    LoaderManager(LoaderManager&&) noexcept = default;
    LoaderManager& operator=(LoaderManager&&) = default;
    ~LoaderManager() = default;

    template <typename T>
    void RegisterLoader(LoaderCallback callback);

    template <typename T>
    LoaderCallback GetLoader() const;

private:
    std::unordered_map<std::type_index, LoaderCallback> loaders_;
};

template <typename T>
inline LoaderManager::LoaderCallback LoaderManager::GetLoader() const {
    auto it = loaders_.find(std::type_index(typeid(T)));
    if (it != loaders_.end()) {
        return it->second;
    }
    return nullptr;
}

template <typename T>
inline void LoaderManager::RegisterLoader(LoaderCallback callback) {
    loaders_[std::type_index(typeid(T))] = std::move(callback);
}

}  // namespace ferrum::resources
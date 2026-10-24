#pragma once

#include <typeinfo>
#include <utility>

#include "ferrum/common/assert.hpp"
#include "ferrum/common/exception.hpp"
#include "ferrum/resources/ResourceEntry.hpp"

namespace ferrum::resources {

template <typename T>
class ResourceHandle {
public:
    ResourceHandle(ResourceEntry* resource_entry) : resource_entry_(resource_entry) {
        if (resource_entry_ == nullptr) {
            return;
        }
        if (resource_entry_->HasResource()) {
            cached_resource_ = CastResource(resource_entry_->resource.get());
        }
        resource_entry_->active_references++;
    }
    ResourceHandle(const ResourceHandle& other)
        : resource_entry_(other.resource_entry_), cached_resource_(other.cached_resource_) {
        AddReference();
    }
    ResourceHandle(ResourceHandle&& other) noexcept
        : resource_entry_(std::exchange(other.resource_entry_, nullptr)),
          cached_resource_(std::exchange(other.cached_resource_, nullptr)) {}
    ResourceHandle& operator=(const ResourceHandle& other) {
        if (this == &other) {
            return *this;
        }
        Release();
        resource_entry_ = other.resource_entry_;
        cached_resource_ = other.cached_resource_;
        AddReference();
        return *this;
    }
    ResourceHandle& operator=(ResourceHandle&& other) noexcept {
        if (this != &other) {
            Release();
            resource_entry_ = std::exchange(other.resource_entry_, nullptr);
            cached_resource_ = std::exchange(other.cached_resource_, nullptr);
        }
        return *this;
    }
    ~ResourceHandle() { Release(); }

    bool IsValid() const { return resource_entry_ != nullptr; }

    T* GetResource() {
        FE_ASSERT(IsValid(), "Resource entry is null");
        if (cached_resource_ != nullptr) {
            return cached_resource_;
        }
        // Can throw on load
        cached_resource_ = CastResource(resource_entry_->Load());
        return cached_resource_;
    }

private:
    static T* CastResource(Resource* resource) {
        if (resource == nullptr) {
            FE_THROW("Loader returned null for requested resource type '{}'", typeid(T).name());
        }
        auto* typed_resource = dynamic_cast<T*>(resource);
        if (typed_resource == nullptr) {
            FE_THROW("Resource type mismatch: requested '{}', loaded '{}'", typeid(T).name(),
                     typeid(*resource).name());
        }
        return typed_resource;
    }

    void AddReference() {
        if (resource_entry_ != nullptr) {
            resource_entry_->active_references++;
        }
    }

    void Release() {
        if (resource_entry_ == nullptr) {
            return;
        }
        ResourceEntry* entry = std::exchange(resource_entry_, nullptr);
        cached_resource_ = nullptr;
        entry->active_references--;
        if (entry->active_references == 0 &&
            entry->policy == ResourceEntry::EvictionPolicy::Discard) {
            entry->Reset();
        }
    }

    ResourceEntry* resource_entry_ = nullptr;
    T* cached_resource_ = nullptr;
};

}  // namespace ferrum::resources
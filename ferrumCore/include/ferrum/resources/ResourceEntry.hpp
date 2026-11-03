#pragma once

#include <functional>
#include <memory>

#include "ferrum/common/exception.hpp"
#include "ferrum/resources/Resource.hpp"

namespace ferrum::resources {

struct ResourceEntry {
    std::function<std::unique_ptr<Resource>()> create;
    std::unique_ptr<Resource> resource;
    // How the resource should be evicted when no longer referenced by any handles
    enum class EvictionPolicy {
        Keep,
        Discard,
    } policy = EvictionPolicy::Discard;

    size_t active_references = 0;

    bool HasResource() const { return resource != nullptr; }
    Resource* GetOrCreate() {
        if (HasResource()) {
            // If already cached by other handle, return it directly
            return resource.get();
        }

        FE_THROW_IF(!create, "Resource creation callback is not set");
        resource = create();
        FE_THROW_IF(resource == nullptr, "Resource creation callback returned null");
        if (policy == EvictionPolicy::Keep) {
            create = nullptr;
        }
        return resource.get();
    }
    void Reset() { resource.reset(); }
};

}  // namespace ferrum::resources

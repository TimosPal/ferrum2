#pragma once

#include <any>
#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>

#include "ferrum/resources/FactoryTraits.hpp"

namespace ferrum::resources {

class FactoryManager {
public:
    template <typename T>
    using FactoryCallback =
        std::function<std::unique_ptr<T>(typename FactoryTraits<T>::Description&)>;

    template <typename T>
    void RegisterFactory(FactoryCallback<T> callback) {
        factories_[std::type_index(typeid(T))] = std::move(callback);
    }

    template <typename T>
    FactoryCallback<T> GetFactory() const {
        auto it = factories_.find(std::type_index(typeid(T)));
        if (it == factories_.end()) {
            return nullptr;
        }
        return std::any_cast<FactoryCallback<T>>(it->second);
    }

private:
    std::unordered_map<std::type_index, std::any> factories_;
};

}  // namespace ferrum::resources

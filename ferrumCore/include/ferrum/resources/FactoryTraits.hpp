#pragma once

namespace ferrum::resources {

template <typename T>
struct FactoryTraits {
    using Description = typename T::Description;
};

}  // namespace ferrum::resources

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

namespace ferrum::containers {

// Dense storage that gives out keys to map's slots based on index.
// If a slot is destroyed the generation is used for invalidation
// Keys are unique per type T so cannot be mixed through different slot maps
template <typename T>
class SlotMap {
public:
    struct Key {
        std::size_t index = 0;
        std::uint32_t generation = 0;

        friend bool operator==(const Key&, const Key&) = default;
        bool IsValid() const { return generation != 0; }
    };

    // Constructs a value in a free slot and returns its handle.
    template <typename... Args>
    Key Create(Args&&... args) {
        const std::size_t index = AllocateSlot();
        Slot& slot = slots_[index];
        slot.value.emplace(std::forward<Args>(args)...);
        ++size_;
        return Key{index, slot.generation};
    }

    // Destroys the value and frees the slot. Returns false if the handle is
    // stale.
    bool Destroy(Key key) {
        Slot* slot = Find(key);
        if (slot == nullptr) {
            return false;
        }
        slot->value.reset();
        ++slot->generation;  // invalidate any other handle to this slot
        free_.push_back(key.index);
        --size_;
        return true;
    }

    // Pointer to the value, or null if the handle is stale.
    T* Get(Key key) { return const_cast<T*>(std::as_const(*this).Get(key)); }
    const T* Get(Key key) const {
        const Slot* slot = Find(key);
        return slot != nullptr ? &*slot->value : nullptr;
    }

    bool Valid(Key key) const { return Find(key) != nullptr; }

    // Number of live values.
    std::size_t Size() const { return size_; }

private:
    struct Slot {
        std::uint32_t generation = 1;
        std::optional<T> value;
    };

    std::size_t AllocateSlot() {
        if (!free_.empty()) {
            const std::size_t index = free_.back();
            free_.pop_back();
            return index;
        }
        slots_.emplace_back();
        return slots_.size() - 1;
    }

    Slot* Find(Key key) { return const_cast<Slot*>(std::as_const(*this).Find(key)); }

    const Slot* Find(Key key) const {
        if (key.index >= slots_.size()) {
            return nullptr;
        }
        const Slot& slot = slots_[key.index];
        if (slot.generation != key.generation || !slot.value.has_value()) {
            return nullptr;
        }
        return &slot;
    }

    std::vector<Slot> slots_;
    std::vector<std::size_t> free_;
    std::size_t size_ = 0;
};

}  // namespace ferrum::containers

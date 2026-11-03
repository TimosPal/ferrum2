#include <gtest/gtest.h>

#include <memory>
#include <ranges>
#include <type_traits>
#include <utility>

#include "ferrum/containers/SlotMap.hpp"

namespace ferrum::containers {
namespace {

using MutableValues = decltype(std::declval<SlotMap<int>&>().Values());
using ConstValues = decltype(std::declval<const SlotMap<int>&>().Values());
static_assert(std::ranges::forward_range<MutableValues>);
static_assert(std::ranges::forward_range<ConstValues>);
static_assert(std::is_same_v<std::ranges::range_reference_t<MutableValues>, int&>);
static_assert(std::is_same_v<std::ranges::range_reference_t<ConstValues>, const int&>);

TEST(SlotMap, CreatesAndRetrievesValues) {
    SlotMap<int> map;
    EXPECT_EQ(map.Size(), 0);
    auto key = map.Create(42);
    EXPECT_TRUE(key.IsValid());
    EXPECT_TRUE(map.Valid(key));
    ASSERT_NE(map.Get(key), nullptr);
    EXPECT_EQ(*map.Get(key), 42);
    EXPECT_EQ(*std::as_const(map).Get(key), 42);
    EXPECT_EQ(map.Size(), 1);
}

TEST(SlotMap, RejectsInvalidAndOutOfBoundsKeys) {
    SlotMap<int> map;
    map.Create(42);
    EXPECT_FALSE(map.Valid({}));
    EXPECT_EQ(map.Get({}), nullptr);
    EXPECT_EQ(map.Get({100, 1}), nullptr);
    EXPECT_FALSE(map.Destroy({100, 1}));
    EXPECT_EQ(map.Size(), 1);
}

TEST(SlotMap, ReusesSlotsWithoutRevivingStaleKeys) {
    SlotMap<int> map;
    auto old_key = map.Create(42);
    EXPECT_TRUE(map.Destroy(old_key));
    EXPECT_FALSE(map.Destroy(old_key));
    EXPECT_EQ(map.Size(), 0);

    auto new_key = map.Create(7);
    EXPECT_EQ(new_key.index, old_key.index);
    EXPECT_NE(new_key.generation, old_key.generation);
    EXPECT_EQ(map.Get(old_key), nullptr);
    ASSERT_NE(map.Get(new_key), nullptr);
    EXPECT_EQ(*map.Get(new_key), 7);
}

TEST(SlotMap, SupportsMoveOnlyValuesAndDestroysThem) {
    SlotMap<std::unique_ptr<int>> map;
    auto key = map.Create(std::make_unique<int>(42));
    ASSERT_NE(map.Get(key), nullptr);
    EXPECT_EQ(**map.Get(key), 42);
    EXPECT_TRUE(map.Destroy(key));
    EXPECT_EQ(map.Get(key), nullptr);
}

TEST(SlotMap, ValuesDoNotVisitEmptyMaps) {
    SlotMap<int> map;
    EXPECT_TRUE(std::ranges::empty(map.Values()));
    EXPECT_TRUE(std::ranges::empty(std::as_const(map).Values()));

    auto key = map.Create(42);
    ASSERT_TRUE(map.Destroy(key));
    EXPECT_TRUE(std::ranges::empty(map.Values()));
    EXPECT_TRUE(std::ranges::empty(std::as_const(map).Values()));
}

TEST(SlotMap, ValuesMutateOnlyLiveValues) {
    SlotMap<int> map;
    auto leading = map.Create(0);
    auto first = map.Create(1);
    auto removed = map.Create(2);
    auto reused = map.Create(3);
    auto trailing = map.Create(5);
    ASSERT_TRUE(map.Destroy(removed));
    ASSERT_TRUE(map.Destroy(reused));
    auto replacement = map.Create(4);
    ASSERT_TRUE(map.Destroy(leading));
    ASSERT_TRUE(map.Destroy(trailing));
    int visits = 0;
    for (auto& value : map.Values()) {
        ++visits;
        value *= 10;
    }

    EXPECT_EQ(visits, 2);
    EXPECT_EQ(*map.Get(first), 10);
    EXPECT_EQ(*map.Get(replacement), 40);
    EXPECT_EQ(map.Get(removed), nullptr);
    EXPECT_EQ(map.Get(reused), nullptr);
    EXPECT_EQ(map.Size(), 2);
}

TEST(SlotMap, ValuesVisitConstValues) {
    SlotMap<int> map;
    map.Create(1);
    auto removed = map.Create(2);
    map.Create(3);
    ASSERT_TRUE(map.Destroy(removed));
    int sum = 0;
    int visits = 0;
    for (const auto& value : std::as_const(map).Values()) {
        ++visits;
        sum += value;
    }
    EXPECT_EQ(visits, 2);
    EXPECT_EQ(sum, 4);
}

TEST(SlotMap, ValuesCanResetMoveOnlyValuesWithoutDestroyingSlots) {
    SlotMap<std::unique_ptr<int>> map;
    auto first = map.Create(std::make_unique<int>(1));
    auto second = map.Create(std::make_unique<int>(2));
    for (auto& value : map.Values()) {
        value.reset();
    }

    EXPECT_TRUE(map.Valid(first));
    EXPECT_TRUE(map.Valid(second));
    EXPECT_EQ(*map.Get(first), nullptr);
    EXPECT_EQ(*map.Get(second), nullptr);
    EXPECT_EQ(map.Size(), 2);
}

}  // namespace
}  // namespace ferrum::containers

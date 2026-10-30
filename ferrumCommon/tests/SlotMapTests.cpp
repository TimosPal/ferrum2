#include <gtest/gtest.h>

#include <memory>
#include <utility>

#include "ferrum/containers/SlotMap.hpp"

namespace ferrum::containers {
namespace {

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

}  // namespace
}  // namespace ferrum::containers

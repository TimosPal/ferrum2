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
    auto oldKey = map.Create(42);
    EXPECT_TRUE(map.Destroy(oldKey));
    EXPECT_FALSE(map.Destroy(oldKey));
    EXPECT_EQ(map.Size(), 0);

    auto newKey = map.Create(7);
    EXPECT_EQ(newKey.index, oldKey.index);
    EXPECT_NE(newKey.generation, oldKey.generation);
    EXPECT_EQ(map.Get(oldKey), nullptr);
    ASSERT_NE(map.Get(newKey), nullptr);
    EXPECT_EQ(*map.Get(newKey), 7);
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

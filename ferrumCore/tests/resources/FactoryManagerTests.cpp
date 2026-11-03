#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "ferrum/resources/FactoryManager.hpp"
#include "ferrum/resources/Resource.hpp"

namespace ferrum::resources {
namespace {

struct NumberResource final : Resource {
    using Description = int;
    explicit NumberResource(int value) : value(value) {}
    int value;
};

struct StringResource final : Resource {
    using Description = std::string;
    explicit StringResource(const std::string& value) : value(value) {}
    std::string value;
};

TEST(FactoryManager, ReturnsEmptyCallbackForUnregisteredResource) {
    FactoryManager manager;
    EXPECT_FALSE(manager.GetFactory<NumberResource>());
}

TEST(FactoryManager, RegistersTypedFactoriesWithoutRunningThem) {
    FactoryManager manager;
    int calls = 0;
    manager.RegisterFactory<NumberResource>([&](int& value) {
        ++calls;
        return std::make_unique<NumberResource>(value);
    });
    auto factory = manager.GetFactory<NumberResource>();
    ASSERT_TRUE(factory);
    EXPECT_EQ(calls, 0);
    int description = 42;
    EXPECT_EQ(factory(description)->value, 42);
    EXPECT_EQ(calls, 1);
}

TEST(FactoryManager, KeepsDifferentResourceAndDescriptionTypesSeparate) {
    FactoryManager manager;
    manager.RegisterFactory<NumberResource>(
        [](int& value) { return std::make_unique<NumberResource>(value); });
    manager.RegisterFactory<StringResource>(
        [](std::string& value) { return std::make_unique<StringResource>(value); });
    int number = 42;
    std::string text = "hello";
    EXPECT_EQ(manager.GetFactory<NumberResource>()(number)->value, 42);
    EXPECT_EQ(manager.GetFactory<StringResource>()(text)->value, "hello");
}

TEST(FactoryManager, ReplacesPreviouslyRegisteredFactory) {
    FactoryManager manager;
    manager.RegisterFactory<NumberResource>(
        [](int& value) { return std::make_unique<NumberResource>(value); });
    manager.RegisterFactory<NumberResource>(
        [](int& value) { return std::make_unique<NumberResource>(value * 2); });
    int description = 21;
    EXPECT_EQ(manager.GetFactory<NumberResource>()(description)->value, 42);
}

TEST(FactoryManager, PreservesEmptyCallbacks) {
    FactoryManager manager;
    manager.RegisterFactory<NumberResource>({});
    EXPECT_FALSE(manager.GetFactory<NumberResource>());
}

}  // namespace
}  // namespace ferrum::resources

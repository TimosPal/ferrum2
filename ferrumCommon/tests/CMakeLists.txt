add_executable(ferrumCommonTests SlotMapTests.cpp)
target_link_libraries(ferrumCommonTests PRIVATE Ferrum::Common GTest::gtest_main)

gtest_discover_tests(ferrumCommonTests
    DISCOVERY_MODE PRE_TEST
    PROPERTIES LABELS "unit"
)

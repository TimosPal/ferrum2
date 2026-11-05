add_executable(ferrumGuiCoreAdapterTests CoreRendererTests.cpp)
target_link_libraries(ferrumGuiCoreAdapterTests PRIVATE Ferrum::GuiCoreAdapter GTest::gtest_main)
gtest_discover_tests(ferrumGuiCoreAdapterTests
    DISCOVERY_MODE PRE_TEST
    PROPERTIES LABELS "unit"
)

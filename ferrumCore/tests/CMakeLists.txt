add_executable(ferrumCoreTests
    resources/FactoryManagerTests.cpp
    resources/ResourceHandleTests.cpp
    resources/ResourceManagerTests.cpp
    resources/ShaderFactoryTests.cpp
    resources/ProgramFactoryTests.cpp
    resources/TextureFactoryTests.cpp
)
target_link_libraries(ferrumCoreTests PRIVATE Ferrum::Core GTest::gtest_main)
target_compile_definitions(ferrumCoreTests PRIVATE
    FERRUM_TEST_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)
gtest_discover_tests(ferrumCoreTests
    DISCOVERY_MODE PRE_TEST
    PROPERTIES LABELS "unit"
)

add_library(ferrumGuiCoreAdapter STATIC
    src/ferrum/gui/CoreRenderer.cpp
)

add_library(Ferrum::GuiCoreAdapter ALIAS ferrumGuiCoreAdapter)

target_include_directories(ferrumGuiCoreAdapter PUBLIC include)

target_compile_definitions(ferrumGuiCoreAdapter PRIVATE
    FERRUM_GUI_CORE_ADAPTER_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)

target_link_libraries(ferrumGuiCoreAdapter PUBLIC Ferrum::GUI Ferrum::Core)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

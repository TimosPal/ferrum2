add_library(ferrumCore STATIC)

add_library(Ferrum::Core ALIAS ferrumCore)

set(FERRUM_CORE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(FERRUM_CORE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(FERRUM_CORE_SOURCES
    ${FERRUM_CORE_SRC_DIR}/ferrum/resources/loaders.cpp
    ${FERRUM_CORE_SRC_DIR}/ferrum/renderer/Renderer.cpp
    ${FERRUM_CORE_SRC_DIR}/ferrum/renderer/backends/OpenGLRenderer.cpp
    ${FERRUM_CORE_SRC_DIR}/ferrum/window/Window.cpp
)

include(cmake/Dependencies.cmake)

if(FERRUM_PLATFORM_BACKEND STREQUAL "GLFW")
    list(APPEND FERRUM_CORE_SOURCES
        ${FERRUM_CORE_SRC_DIR}/ferrum/core/backends/GLFWPlatform.cpp
        ${FERRUM_CORE_SRC_DIR}/ferrum/graphics/backends/GLFWOpenGLWindowBinding.cpp
        ${FERRUM_CORE_SRC_DIR}/ferrum/window/backends/GLFWWindow.cpp
    )
    target_link_libraries(ferrumCore PRIVATE glfw)
else()
    message(FATAL_ERROR "Unsupported Ferrum platform backend: ${FERRUM_PLATFORM_BACKEND}")
endif()

target_sources(ferrumCore PRIVATE ${FERRUM_CORE_SOURCES})

target_include_directories(ferrumCore
    PUBLIC ${FERRUM_CORE_INCLUDE_DIR}
    PRIVATE ${FERRUM_CORE_SRC_DIR}
)

target_link_libraries(ferrumCore
    PUBLIC Ferrum::Common glm::glm
    PRIVATE stb_image freetype glad_gl_core_46
)

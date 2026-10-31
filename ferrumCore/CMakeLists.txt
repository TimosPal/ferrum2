add_library(ferrumCore STATIC)

add_library(Ferrum::Core ALIAS ferrumCore)

include(cmake/Dependencies.cmake)

target_sources(ferrumCore PRIVATE
    src/ferrum/resources/loaders.cpp
    src/ferrum/graphics/GraphicsDevice.cpp
    src/ferrum/renderer/backends/opengl/GLBuffer.cpp
    src/ferrum/renderer/backends/opengl/GLMesh.cpp
    src/ferrum/renderer/Camera2D.cpp
    src/ferrum/renderer/Material.cpp
    src/ferrum/renderer/LayoutDescriptor.cpp
    src/ferrum/renderer/backends/opengl/GLProgram.cpp
    src/ferrum/renderer/backends/opengl/GLRenderer.cpp
    src/ferrum/renderer/backends/opengl/GLGraphicsDevice.cpp
    src/ferrum/renderer/backends/opengl/GLShader.cpp
    src/ferrum/renderer/backends/opengl/GLVertexArray.cpp
    src/ferrum/window/Window.cpp
)

if(FERRUM_PLATFORM_BACKEND STREQUAL "GLFW")
    target_sources(ferrumCore PRIVATE
        src/ferrum/core/backends/glfw/GLFWPlatform.cpp
        src/ferrum/graphics/backends/glfw/GLFWOpenGLWindowBinding.cpp
        src/ferrum/window/backends/glfw/GLFWWindow.cpp
    )
    target_link_libraries(ferrumCore PRIVATE glfw)
else()
    message(FATAL_ERROR "Unsupported Ferrum platform backend: ${FERRUM_PLATFORM_BACKEND}")
endif()

target_include_directories(ferrumCore
    PUBLIC include
    PRIVATE src
)

# Development assets are loaded directly from the source tree, independent of the working directory.
target_compile_definitions(ferrumCore PUBLIC
    FERRUM_CORE_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)

target_link_libraries(ferrumCore
    PUBLIC Ferrum::Common glm::glm
    PRIVATE stb_image freetype glad_gl_core_46
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

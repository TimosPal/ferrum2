cmake_minimum_required(VERSION 3.30)

project(Ferrum LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(FERRUM_PLATFORM_BACKEND "GLFW" CACHE STRING "Platform backend used by Ferrum Core")
set_property(CACHE FERRUM_PLATFORM_BACKEND PROPERTY STRINGS GLFW)

include(CTest)

include(cmake/ClangTidy.cmake)

if(BUILD_TESTING)
    include(cmake/Dependencies.cmake)
    include(GoogleTest)
endif()

add_subdirectory(ferrumCommon)
add_subdirectory(ferrumCore)
add_subdirectory(ferrumGUI)
add_subdirectory(ferrumSandbox)

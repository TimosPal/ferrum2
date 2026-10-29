include(FetchContent)

# GoogleTest (shared by module-local test suites).
if(MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(googletest)

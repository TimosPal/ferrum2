# Runs clang-tidy on every C++ compile when FERRUM_ENABLE_CLANG_TIDY is ON.
# Checks come from the repository's .clang-tidy file.

option(FERRUM_ENABLE_CLANG_TIDY "Run clang-tidy during compilation" OFF)

if(NOT FERRUM_ENABLE_CLANG_TIDY)
    return()
endif()

find_program(FERRUM_CLANG_TIDY_EXE NAMES clang-tidy)
if(NOT FERRUM_CLANG_TIDY_EXE)
    message(FATAL_ERROR "FERRUM_ENABLE_CLANG_TIDY is ON but clang-tidy was not found on PATH")
endif()

set(CMAKE_CXX_CLANG_TIDY "${FERRUM_CLANG_TIDY_EXE}")

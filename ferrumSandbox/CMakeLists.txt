add_executable(ferrumSandbox src/main.cpp)

target_link_libraries(ferrumSandbox PRIVATE Ferrum::Core Ferrum::Common)

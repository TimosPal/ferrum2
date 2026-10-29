add_library(ferrumCommon INTERFACE)

add_library(Ferrum::Common ALIAS ferrumCommon)

target_include_directories(ferrumCommon INTERFACE include)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

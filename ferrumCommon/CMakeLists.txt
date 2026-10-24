add_library(ferrumCommon INTERFACE)

add_library(Ferrum::Common ALIAS ferrumCommon)

target_include_directories(ferrumCommon INTERFACE include)

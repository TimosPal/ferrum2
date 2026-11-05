add_library(ferrumGUI INTERFACE)

add_library(Ferrum::GUI ALIAS ferrumGUI)

target_include_directories(ferrumGUI INTERFACE include)
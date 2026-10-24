#pragma once

#include <string>

namespace ferrum::window {

struct WindowDesc {
    std::string title;
    int width = 0;
    int height = 0;
    bool vsync = false;
};

}  // namespace ferrum::window
#pragma once

#include <glm/glm.hpp>

namespace ferrum::renderer {

struct CameraMatrices {
    glm::mat4 view{1.0F};
    glm::mat4 projection{1.0F};
    glm::mat4 view_projection{1.0F};
};

}  // namespace ferrum::renderer

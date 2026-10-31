#pragma once

#include <glm/glm.hpp>

namespace ferrum::renderer {

class Camera2D {
public:
    explicit Camera2D(float aspect_ratio = 1.0F) : aspect_ratio_(aspect_ratio) {}

    const glm::vec2& GetPosition() const noexcept { return position_; }
    float GetZoom() const noexcept { return zoom_; }
    float GetAspectRatio() const noexcept { return aspect_ratio_; }

    void SetPosition(glm::vec2 position) noexcept;
    void SetZoom(float zoom) noexcept;
    void SetAspectRatio(float aspect_ratio) noexcept;

    glm::mat4 GetView() const noexcept;
    glm::mat4 GetProjection() const noexcept;
    glm::mat4 GetViewProjection() const noexcept;

private:
    void UpdateMatrices() const noexcept;

    glm::vec2 position_{0.0F};
    float zoom_ = 1.0F;
    float aspect_ratio_ = 1.0F;
    mutable glm::mat4 view_{1.0F};
    mutable glm::mat4 projection_{1.0F};
    mutable glm::mat4 view_projection_{1.0F};
    mutable bool view_projection_dirty_ = true;
};

}  // namespace ferrum::renderer

#include "ferrum/renderer/Camera2D.hpp"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

namespace ferrum::renderer {

void Camera2D::SetPosition(glm::vec2 position) noexcept {
    if (position_ != position) {
        position_ = position;
        view_projection_dirty_ = true;
    }
}

void Camera2D::SetZoom(float zoom) noexcept {
    const auto clamped_zoom = std::max(zoom, 0.01F);
    if (zoom_ != clamped_zoom) {
        zoom_ = clamped_zoom;
        view_projection_dirty_ = true;
    }
}

void Camera2D::SetAspectRatio(float aspect_ratio) noexcept {
    const auto clamped_aspect_ratio = std::max(aspect_ratio, 0.01F);
    if (aspect_ratio_ != clamped_aspect_ratio) {
        aspect_ratio_ = clamped_aspect_ratio;
        view_projection_dirty_ = true;
    }
}

void Camera2D::UpdateMatrices() const noexcept {
    if (!view_projection_dirty_) {
        return;
    }

    const float half_height = zoom_;
    const float half_width = half_height * aspect_ratio_;
    matrices_.projection =
        glm::ortho(-half_width, half_width, -half_height, half_height, -1.0F, 1.0F);
    matrices_.view = glm::translate(glm::mat4(1.0F), glm::vec3(-position_.x, -position_.y, 0.0F));
    matrices_.view_projection = matrices_.projection * matrices_.view;
    view_projection_dirty_ = false;
}

const glm::mat4& Camera2D::GetView() const noexcept {
    UpdateMatrices();
    return matrices_.view;
}

const glm::mat4& Camera2D::GetProjection() const noexcept {
    UpdateMatrices();
    return matrices_.projection;
}

const glm::mat4& Camera2D::GetViewProjection() const noexcept {
    UpdateMatrices();
    return matrices_.view_projection;
}

const CameraMatrices& Camera2D::GetMatrices() const noexcept {
    UpdateMatrices();
    return matrices_;
}

}  // namespace ferrum::renderer

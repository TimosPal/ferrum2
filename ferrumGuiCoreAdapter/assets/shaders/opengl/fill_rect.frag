#version 460 core

layout(location = 0) in vec2 localPosition;
layout(location = 0) out vec4 fragmentColor;

uniform vec4 color;
uniform vec2 rectSize;
uniform vec4 cornerRadii;

void main() {
    bool left = localPosition.x < 0.0;
    bool top = localPosition.y < 0.0;
    float radius = top ? (left ? cornerRadii.x : cornerRadii.y)
                        : (left ? cornerRadii.w : cornerRadii.z);
    vec2 halfSize = rectSize * 0.5;

    vec2 pixelPosition = localPosition * rectSize;
    vec2 center = vec2(left ? -halfSize.x + radius : halfSize.x - radius,
                       top ? -halfSize.y + radius : halfSize.y - radius);
    float distanceFromCenter = length(pixelPosition - center);
    bool inCornerX = left ? pixelPosition.x < center.x : pixelPosition.x > center.x;
    bool inCornerY = top ? pixelPosition.y < center.y : pixelPosition.y > center.y;
    if (inCornerX && inCornerY && distanceFromCenter > radius) {
        discard;
    }

    fragmentColor = color;
}

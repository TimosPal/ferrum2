#version 460 core

layout(location = 0) in vec2 position;

layout(location = 0) out vec2 localPosition;

uniform mat4 viewProjection;
uniform mat4 model;

void main() {
    localPosition = position;
    gl_Position = viewProjection * model * vec4(position, 0.0, 1.0);
}

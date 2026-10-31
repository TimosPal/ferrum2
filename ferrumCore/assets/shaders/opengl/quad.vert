#version 460 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 tint;

uniform mat4 viewProjection;

out vec2 fragmentUv;
out vec4 fragmentTint;

void main() {
    gl_Position = viewProjection * vec4(position, 0.0, 1.0);
    fragmentUv = uv;
    fragmentTint = tint;
}

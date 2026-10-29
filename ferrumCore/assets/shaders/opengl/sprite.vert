#version 460 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 textureCoordinates;

uniform mat4 model;
uniform mat4 viewProjection;

out vec2 fragmentTextureCoordinates;

void main() {
    fragmentTextureCoordinates = textureCoordinates;
    gl_Position = viewProjection * model * vec4(position, 0.0, 1.0);
}

#version 460 core

layout(location = 0) out vec4 fragmentColor;

in vec2 fragmentUv;
in vec4 fragmentTint;

uniform sampler2D spriteTexture;

void main() {
    fragmentColor = texture(spriteTexture, fragmentUv) * fragmentTint;
}

#version 460 core

in vec2 fragmentTextureCoordinates;

uniform sampler2D spriteTexture;
uniform vec4 tint;

layout(location = 0) out vec4 fragmentColor;

void main() {
    fragmentColor = texture(spriteTexture, fragmentTextureCoordinates) * tint;
}

#version 330 core

in vec2 vTexCoord;
out vec4 fragColor;

uniform vec4 uColor;
uniform sampler2D uTexture;
uniform bool uUseTexture;

void main() {
    fragColor = uColor * (uUseTexture ? texture(uTexture, vTexCoord) : vec4(1.0));
}

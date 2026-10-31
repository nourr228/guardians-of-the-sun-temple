#version 330 core

in vec2 vTexCoord;
out vec4 fragColor;

uniform sampler2D uTexture;
uniform vec4 uColor;

void main() {
    fragColor = vec4(uColor.rgb, uColor.a * texture(uTexture, vTexCoord).r);
}

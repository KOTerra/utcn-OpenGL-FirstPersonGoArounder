#version 410 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D rainTexture;
uniform float time;

void main() {
    // Scroll texture downwards (v - time)
    vec2 animCoords = vec2(TexCoords.x, TexCoords.y + (time * 1.f));

    vec4 color = texture(rainTexture, animCoords);

    // Discard transparent parts
    if (color.a < 0.1)
    discard;

    FragColor = color;
}
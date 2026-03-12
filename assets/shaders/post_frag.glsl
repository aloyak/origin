#version 410 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float colorLevels;

void main() {
    vec3 color = texture(screenTexture, TexCoords).rgb;
    
    if (colorLevels > 0.0) {
        color = floor(color * colorLevels) / colorLevels;
    }
    
    FragColor = vec4(color, 1.0);
}
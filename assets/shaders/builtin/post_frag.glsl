#version 410 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float colorLevels;

uniform bool u_GammaEnabled;
uniform float u_Gamma;
uniform float u_Exposure;

void main() {
    vec3 color = texture(screenTexture, TexCoords).rgb;

    color = vec3(1.0) - exp(-color * u_Exposure);

    if (u_GammaEnabled) {
        color = pow(max(color, vec3(0.0)), vec3(1.0 / max(u_Gamma, 0.01)));
    }
    
    if (colorLevels > 0.0) {
        color = floor(color * colorLevels) / colorLevels;
    }
    
    FragColor = vec4(color, 1.0);
}
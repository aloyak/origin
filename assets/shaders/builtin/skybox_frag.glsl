#version 410 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;
uniform vec3 colorTint;

void main() {    
    FragColor = texture(skybox, TexCoords) * vec4(colorTint, 1.0);
}
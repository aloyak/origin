#version 420 core
out vec4 FragColor;

in vec3 mColor;
in vec2 TexCoord;

uniform sampler2D u_Texture;

void main() {
    FragColor = texture(u_Texture, TexCoord) * vec4(mColor, 1.0);   
}
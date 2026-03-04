#version 420 core
out vec4 FragColor;

in vec3 vNormal;
in vec2 TexCoord;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
};

uniform Material material;

void main() {
    vec4 diffuseColor = texture(material.texture_diffuse1, TexCoord);
    
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * diffuseColor.rgb;

    FragColor = vec4(ambient, diffuseColor.a);
}
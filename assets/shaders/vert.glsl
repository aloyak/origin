#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 vNormal;
out vec2 TexCoord;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

uniform bool u_VertexSnap;
uniform float u_SnapIntensity;

void main() {
    vec4 pos = u_Projection * u_View * u_Model * vec4(aPos, 1.0);
    
    if (u_VertexSnap) {
        pos.xyz = pos.xyz / pos.w; 
        pos.xy = floor(pos.xy * u_SnapIntensity) / u_SnapIntensity;
        pos.xyz *= pos.w;
    }

    gl_Position = pos;
    TexCoord = aTexCoord;
    vNormal = mat3(transpose(inverse(u_Model))) * aNormal;
}
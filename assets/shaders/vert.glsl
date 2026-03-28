#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec3 vNormal;
out vec3 vWorldPos;
out vec2 TexCoord;
out mat3 vTBN;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

uniform bool u_VertexSnap;
uniform float u_SnapIntensity;

void main() {
    vec4 worldPos = u_Model * vec4(aPos, 1.0);
    vec4 pos = u_Projection * u_View * worldPos;
    
    if (u_VertexSnap) {
        pos.xyz = pos.xyz / pos.w; 
        pos.xy = floor(pos.xy * u_SnapIntensity) / u_SnapIntensity;
        pos.xyz *= pos.w;
    }

    gl_Position = pos;
    vWorldPos = worldPos.xyz;
    TexCoord = aTexCoord;

    mat3 normalMat = mat3(transpose(inverse(u_Model)));
    vec3 N = normalize(normalMat * aNormal);
    vec3 T = normalize(normalMat * aTangent);
    T = normalize(T - dot(T, N) * N);
    vec3 B = normalize(cross(N, T));

    vNormal = N;
    vTBN = mat3(T, B, N);
}
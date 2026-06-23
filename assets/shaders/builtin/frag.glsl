#version 410 core

#define MAX_DIR_LIGHTS 4
#define MAX_POINT_LIGHTS 8

out vec4 FragColor;

in vec3 vNormal;
in vec3 vWorldPos;
in vec2 TexCoord;
in mat3 vTBN;

struct DirLight { 
    vec3 direction; 
    vec3 color; 
    float intensity; 
};

struct PointLight { 
    vec3 position;
    vec3 color; 
    float intensity; 
    float radius; 
};

struct Material {
    sampler2D texture_diffuse;
    sampler2D texture_specular;
    sampler2D texture_normal;
    sampler2D texture_metallic;
    // TODO (ideas):
    // sampler2D texture_roughness;
    // sampler2D texture_ao;
    // sampler2D texture_emissive;
    // sampler2D texture_height;
};

uniform Material material;

// Lighting uniforms
uniform vec3 u_ViewPos;
uniform DirLight dirLights[MAX_DIR_LIGHTS];
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numDirLights;
uniform int numPointLights;
uniform bool u_LightingEnabled;
uniform float u_MinAmbientLight;

// Material properties
uniform float u_AmbientStrength;
uniform float u_SpecularStrength;
uniform float u_Shininess;
uniform vec3 u_BaseColor;
uniform vec2 u_UVScale;

uniform bool u_NormalMap;
uniform bool u_MetallicMap;

vec3 srgbToLinear(vec3 color) {
    return pow(max(color, vec3(0.0)), vec3(2.2));
}

vec3 calcDirLight(DirLight light, vec3 norm, vec3 viewDir, vec3 diffuseColor, vec3 specularColor) {
    vec3 lightDir = normalize(-light.direction);  // negative because direction points FROM light

    float NdotL = dot(norm, lightDir);

    // Diffuse
    float diff = max(NdotL, 0.0);
    vec3 diffuse = diff * diffuseColor * light.color * light.intensity;

    // Blinn-Phong
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfDir), 0.0), u_Shininess);
    spec *= smoothstep(0.0, 0.1, NdotL);
    vec3 specular = u_SpecularStrength * spec * specularColor * light.color * light.intensity;

    return diffuse + specular;
}

vec3 calcPointLight(PointLight light, vec3 norm, vec3 fragPos, vec3 viewDir, vec3 diffuseColor, vec3 specularColor) {
    vec3 toLight = light.position - fragPos;
    float distanceToLight = length(toLight);
    vec3 lightDir = normalize(toLight);

    float radius = max(light.radius, 0.001);
    float attenuation = clamp(1.0 - (distanceToLight / radius), 0.0, 1.0);

    float NdotL = dot(norm, lightDir);

    float diff = max(NdotL, 0.0);
    vec3 diffuse = diff * diffuseColor * light.color * light.intensity;

    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfDir), 0.0), u_Shininess);
    spec *= smoothstep(0.0, 0.1, NdotL);
    vec3 specular = u_SpecularStrength * spec * specularColor * light.color * light.intensity;

    return (diffuse + specular) * attenuation;
}

void main() {
    vec2 tiledTexCoord = TexCoord * u_UVScale;
    vec4 diffuseTexture = texture(material.texture_diffuse, tiledTexCoord);
    vec4 specularTexture = texture(material.texture_specular, tiledTexCoord);

    vec3 diffuseColor = srgbToLinear(diffuseTexture.rgb) * u_BaseColor;
    float metallic = 0.0;
    if (u_MetallicMap) {
        metallic = clamp(texture(material.texture_metallic, tiledTexCoord).r, 0.0, 1.0);
    }

    vec3 specularColor = mix(specularTexture.rgb, diffuseColor, metallic);
    diffuseColor *= (1.0 - metallic);

    if (!u_LightingEnabled) {
        FragColor = vec4(diffuseColor, diffuseTexture.a);
        return;
    }
    
    vec3 norm = normalize(vNormal);
    if (u_NormalMap) {
        vec3 sampled = texture(material.texture_normal, tiledTexCoord).rgb;
        sampled = sampled * 2.0 - 1.0;
        norm = normalize(vTBN * sampled);
    }
    vec3 viewDir = normalize(u_ViewPos - vWorldPos);
    
    // Global ambient floor keeps objects visible even with no active directional lights.
    float ambientLevel = max(u_AmbientStrength, u_MinAmbientLight);
    vec3 result = ambientLevel * diffuseColor;

    // Calculate lighting from all directional lights
    for(int i = 0; i < numDirLights && i < MAX_DIR_LIGHTS; i++) {
        result += calcDirLight(dirLights[i], norm, viewDir, diffuseColor, specularColor);
    }

    for (int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++) {
        result += calcPointLight(pointLights[i], norm, vWorldPos, viewDir, diffuseColor, specularColor);
    }
    
    FragColor = vec4(result, diffuseTexture.a);
}
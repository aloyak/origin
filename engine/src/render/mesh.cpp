#ifdef __EMSCRIPTEN__
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif

#include "engine/render/texture.h"
#include "engine/render/mesh.h"
#include "engine/render/shader.h"
#include "engine/render/material.h"
#include "engine/lighting/directionalLight.h"
#include "engine/lighting/pointLight.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<MeshTexture> textures, bool dynamic, Vec3 baseColor) {
    m_vertexCount = vertices.size(); 
    m_indexCount  = indices.size();
    m_dynamic     = dynamic;
    m_baseColor   = baseColor;

    this->vertices = vertices;
    this->indices  = indices;
    this->textures = textures;

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);
  
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0],
                 m_dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);  

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    glEnableVertexAttribArray(1);	
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    glEnableVertexAttribArray(2);	
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

    glBindVertexArray(0);
}

Mesh::~Mesh() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_ebo) glDeleteBuffers(1, &m_ebo);
}

Mesh::Mesh(Mesh&& other) noexcept
    : vertices(std::move(other.vertices))
    , indices(std::move(other.indices))
    , textures(std::move(other.textures))
    , m_vao(other.m_vao)
    , m_vbo(other.m_vbo)
    , m_ebo(other.m_ebo)
    , m_vertexCount(other.m_vertexCount)
    , m_indexCount(other.m_indexCount)
    , m_dynamic(other.m_dynamic)
    , m_baseColor(other.m_baseColor)
{
    other.m_vao = 0;
    other.m_vbo = 0;
    other.m_ebo = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this == &other) return *this;

    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_ebo) glDeleteBuffers(1, &m_ebo);

    vertices = std::move(other.vertices);
    indices  = std::move(other.indices);
    textures = std::move(other.textures);

    m_vao = other.m_vao;
    m_vbo = other.m_vbo;
    m_ebo = other.m_ebo;
    m_vertexCount = other.m_vertexCount;
    m_indexCount  = other.m_indexCount;
    m_dynamic     = other.m_dynamic;
    m_baseColor   = other.m_baseColor;

    other.m_vao = 0;
    other.m_vbo = 0;
    other.m_ebo = 0;

    return *this;
}

void Mesh::updateVertexBuffer() {
    if (!m_dynamic) {
        return; 
    }
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::recalculateNormals() {
    for (auto& v : vertices) {
        v.Normal = Vec3(0.0f, 0.0f, 0.0f);
    }

    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        unsigned int i0 = indices[i];
        unsigned int i1 = indices[i + 1];
        unsigned int i2 = indices[i + 2];

        const Vec3& p0 = vertices[i0].Position;
        const Vec3& p1 = vertices[i1].Position;
        const Vec3& p2 = vertices[i2].Position;

        Vec3 e1 = p1 - p0;
        Vec3 e2 = p2 - p0;

        Vec3 faceNormal(
            e1.y * e2.z - e1.z * e2.y,
            e1.z * e2.x - e1.x * e2.z,
            e1.x * e2.y - e1.y * e2.x
        );

        vertices[i0].Normal = vertices[i0].Normal + faceNormal;
        vertices[i1].Normal = vertices[i1].Normal + faceNormal;
        vertices[i2].Normal = vertices[i2].Normal + faceNormal;
    }

    for (auto& v : vertices) {
        float lenSq = v.Normal.x * v.Normal.x + v.Normal.y * v.Normal.y + v.Normal.z * v.Normal.z;
        if (lenSq > 1e-12f) {
            v.Normal = v.Normal.normalize();
        } else {
            v.Normal = Vec3(0.0f, 0.0f, 1.0f);
        }
    }
}

void Mesh::draw(const Material& material,
                const std::vector<DirectionalLight>& directionalLights,
                const std::vector<PointLight>& pointLights) const {
    Shader& shader = material.getShader();

    unsigned int textureUnit = 0;

    auto bindTextureSlot = [&](const std::string& uniformName, const std::string& meshType) -> bool {
        std::shared_ptr<Texture> selected = material.getTexture(uniformName);

        if (!selected) {
            for (const auto& meshTexture : textures) {
                if (meshTexture.type == meshType && meshTexture.texture) {
                    selected = meshTexture.texture;
                    break;
                }
            }
        }

        if (!selected) {
            return false;
        }

        shader.setInt(("material." + uniformName).c_str(), textureUnit);
        selected->bind(textureUnit);
        ++textureUnit;
        return true;
    };

    bool hasDiffuseMap   = bindTextureSlot("texture_diffuse", "texture_diffuse");
    bool hasSpecularMap  = bindTextureSlot("texture_specular", "texture_specular");
    bool hasNormalMap    = bindTextureSlot("texture_normal", "texture_normal");
    bool hasMetallicMap  = bindTextureSlot("texture_metallic", "texture_metallic");

    shader.setBool("u_DiffuseMap", hasDiffuseMap);
    shader.setBool("u_SpecularMap", hasSpecularMap);
    shader.setBool("u_NormalMap", hasNormalMap);
    shader.setBool("u_MetallicMap", hasMetallicMap);

    Vec3 effectiveBaseColor = material.getBaseColor();
    bool materialColorIsDefault = (effectiveBaseColor.x == 1.0f && effectiveBaseColor.y == 1.0f && effectiveBaseColor.z == 1.0f);
    if (!hasDiffuseMap && materialColorIsDefault) {
        effectiveBaseColor = m_baseColor;
    }

    // Bind material properties
    shader.setFloat("u_AmbientStrength", material.getAmbientStrength());
    shader.setFloat("u_SpecularStrength", material.getSpecularStrength());
    shader.setFloat("u_Shininess", material.getShininess());
    shader.setVec3("u_BaseColor", effectiveBaseColor);
    shader.setVec2("u_UVScale", material.getUVScale());
    
    // Bind directional light uniforms
    for (size_t i = 0; i < directionalLights.size() && i < m_maxDirectionalLights; ++i) {
        const auto& light = directionalLights[i];
        std::string prefix = "dirLights[" + std::to_string(i) + "]";
        
        shader.setVec3((prefix + ".direction").c_str(), light.getDirection());
        shader.setVec3((prefix + ".color").c_str(), light.getColor());
        shader.setFloat((prefix + ".intensity").c_str(), light.getIntensity());
    }
    
    shader.setInt("numDirLights", static_cast<int>(std::min(directionalLights.size(), size_t(m_maxDirectionalLights))));

    for (size_t i = 0; i < pointLights.size() && i < m_maxPointLights; ++i) {
        const auto& light = pointLights[i];
        std::string prefix = "pointLights[" + std::to_string(i) + "]";

        shader.setVec3((prefix + ".position").c_str(), light.getPosition());
        shader.setVec3((prefix + ".color").c_str(), light.getColor());
        shader.setFloat((prefix + ".intensity").c_str(), light.getIntensity());
        shader.setFloat((prefix + ".radius").c_str(), light.getRadius());
    }

    shader.setInt("numPointLights", static_cast<int>(std::min(pointLights.size(), size_t(m_maxPointLights))));
    
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Unbind all texture units that were used
    for (unsigned int i = 0; i < textureUnit; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glActiveTexture(GL_TEXTURE0);
}

void Mesh::drawInstanced(const Material& material,
                         const std::vector<DirectionalLight>& directionalLights,
                         const std::vector<PointLight>& pointLights,
                         unsigned int instanceVBO,
                         unsigned int instanceCount) const {
    Shader& materialShader = material.getShader();

    unsigned int textureUnit = 0;

    auto bindTextureSlot = [&](const std::string& uniformName, const std::string& meshType) -> bool {
        std::shared_ptr<Texture> selected = material.getTexture(uniformName);

        if (!selected) {
            for (const auto& meshTexture : textures) {
                if (meshTexture.type == meshType && meshTexture.texture) {
                    selected = meshTexture.texture;
                    break;
                }
            }
        }

        if (!selected) {
            return false;
        }

        materialShader.setInt(("material." + uniformName).c_str(), textureUnit);
        selected->bind(textureUnit);
        ++textureUnit;
        return true;
    };

    bool hasDiffuseMap   = bindTextureSlot("texture_diffuse", "texture_diffuse");
    bool hasSpecularMap  = bindTextureSlot("texture_specular", "texture_specular");
    bool hasNormalMap    = bindTextureSlot("texture_normal", "texture_normal");
    bool hasMetallicMap  = bindTextureSlot("texture_metallic", "texture_metallic");

    materialShader.setBool("u_DiffuseMap", hasDiffuseMap);
    materialShader.setBool("u_SpecularMap", hasSpecularMap);
    materialShader.setBool("u_NormalMap", hasNormalMap);
    materialShader.setBool("u_MetallicMap", hasMetallicMap);

    Vec3 effectiveBaseColor = material.getBaseColor();
    bool materialColorIsDefault = (effectiveBaseColor.x == 1.0f && effectiveBaseColor.y == 1.0f && effectiveBaseColor.z == 1.0f);
    if (!hasDiffuseMap && materialColorIsDefault) {
        effectiveBaseColor = m_baseColor;
    }

    materialShader.setFloat("u_AmbientStrength", material.getAmbientStrength());
    materialShader.setFloat("u_SpecularStrength", material.getSpecularStrength());
    materialShader.setFloat("u_Shininess", material.getShininess());
    materialShader.setVec3("u_BaseColor", effectiveBaseColor);
    materialShader.setVec2("u_UVScale", material.getUVScale());
    
    for (size_t i = 0; i < directionalLights.size() && i < m_maxDirectionalLights; ++i) {
        const auto& light = directionalLights[i];
        std::string prefix = "dirLights[" + std::to_string(i) + "]";
        
        materialShader.setVec3((prefix + ".direction").c_str(), light.getDirection());
        materialShader.setVec3((prefix + ".color").c_str(), light.getColor());
        materialShader.setFloat((prefix + ".intensity").c_str(), light.getIntensity());
    }
    
    materialShader.setInt("numDirLights", static_cast<int>(std::min(directionalLights.size(), size_t(m_maxDirectionalLights))));

    for (size_t i = 0; i < pointLights.size() && i < m_maxPointLights; ++i) {
        const auto& light = pointLights[i];
        std::string prefix = "pointLights[" + std::to_string(i) + "]";

        materialShader.setVec3((prefix + ".position").c_str(), light.getPosition());
        materialShader.setVec3((prefix + ".color").c_str(), light.getColor());
        materialShader.setFloat((prefix + ".intensity").c_str(), light.getIntensity());
        materialShader.setFloat((prefix + ".radius").c_str(), light.getRadius());
    }

    materialShader.setInt("numPointLights", static_cast<int>(std::min(pointLights.size(), size_t(m_maxPointLights))));
    
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

    std::size_t vec4Size = 4 * sizeof(float);
    std::size_t mat4Size = 4 * vec4Size;

    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, mat4Size, (void*)0);
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, mat4Size, (void*)(1 * vec4Size));
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, mat4Size, (void*)(2 * vec4Size));
    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, mat4Size, (void*)(3 * vec4Size));

    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);
    glVertexAttribDivisor(7, 1);
    glVertexAttribDivisor(8, 1);

    glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0, instanceCount);
    
    glVertexAttribDivisor(5, 0);
    glVertexAttribDivisor(6, 0);
    glVertexAttribDivisor(7, 0);
    glVertexAttribDivisor(8, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    for (unsigned int i = 0; i < textureUnit; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glActiveTexture(GL_TEXTURE0);
}
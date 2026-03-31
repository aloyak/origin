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

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<MeshTexture> textures) {
    m_vertexCount = vertices.size(); 
    m_indexCount  = indices.size();

    this->vertices = vertices;
    this->indices  = indices;
    this->textures = textures;

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);
  
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);  

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

    other.m_vao = 0;
    other.m_vbo = 0;
    other.m_ebo = 0;

    return *this;
}

void Mesh::draw(const Material& material,
                const std::vector<DirectionalLight>& directionalLights,
                const std::vector<PointLight>& pointLights) const {
    Shader& shader = material.getShader();

    unsigned int textureUnit = 0;

    auto bindTextureSlot = [&](const std::string& uniformName, const std::string& meshType) {
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
            return;
        }

        shader.setInt(("material." + uniformName).c_str(), textureUnit);
        selected->bind(textureUnit);
        ++textureUnit;
    };

    bindTextureSlot("texture_diffuse", "texture_diffuse");
    bindTextureSlot("texture_specular", "texture_specular");
    bindTextureSlot("texture_normal", "texture_normal");
    bindTextureSlot("texture_metallic", "texture_metallic");

    bool hasNormalMap = material.hasTexture("texture_normal");
    if (!hasNormalMap) {
        for (const auto& meshTexture : textures) {
            if (meshTexture.type == "texture_normal" && meshTexture.texture) {
                hasNormalMap = true;
                break;
            }
        }
    }
    shader.setBool("u_NormalMap", hasNormalMap);

    bool hasMetallicMap = material.hasTexture("texture_metallic");
    if (!hasMetallicMap) {
        for (const auto& meshTexture : textures) {
            if (meshTexture.type == "texture_metallic" && meshTexture.texture) {
                hasMetallicMap = true;
                break;
            }
        }
    }
    shader.setBool("u_MetallicMap", hasMetallicMap);

    // Bind material properties
    shader.setFloat("u_AmbientStrength", material.getAmbientStrength());
    shader.setFloat("u_SpecularStrength", material.getSpecularStrength());
    shader.setFloat("u_Shininess", material.getShininess());
    shader.setVec3("u_BaseColor", material.getBaseColor());
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
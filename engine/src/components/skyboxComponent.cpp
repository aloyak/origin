#include "engine/components/skyboxComponent.h"
#include "engine/render/camera.h"
#include "engine/engine.h"

#include "engine/utils/logger.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 

#ifdef __EMSCRIPTEN__
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif

#include <stb_image.h>
#include <nlohmann/json.hpp>

#include "engine/utils/path.h"

namespace {
    bool hasCompleteFaces(const std::vector<std::string>& faces) {
        if (faces.size() != 6) {
            return false;
        }
        for (const auto& face : faces) {
            if (face.empty()) {
                return false;
            }
        }
        return true;
    }

    std::vector<std::string> normalizeFaces(const std::vector<std::string>& faces) {
        std::vector<std::string> normalized = faces;
        for (auto& face : normalized) {
            face = Path::toAssetsRelative(face);
        }
        return normalized;
    }
}

SkyboxComponent::SkyboxComponent(const std::vector<std::string>& faces) 
    : m_facePaths(faces.empty() ? std::vector<std::string>(6) : normalizeFaces(faces)) {
    m_shader = std::make_unique<Shader>("assets/shaders/builtin/skybox_vert.glsl", "assets/shaders/builtin/skybox_frag.glsl");
    setupMesh();
    loadCubemap(m_facePaths);
}

std::unique_ptr<Component> SkyboxComponent::clone() const {
    auto copy = std::make_unique<SkyboxComponent>(m_facePaths);
    copy->setRotation(m_rotation);
    copy->isEnabled = isEnabled;
    return copy;
}

void SkyboxComponent::loadCubemap(const std::vector<std::string>& faces) {
    if (m_cubemapID != 0) {
        glDeleteTextures(1, &m_cubemapID);
        m_cubemapID = 0;
    }

    if (!hasCompleteFaces(faces)) {
        return;
    }

    glGenTextures(1, &m_cubemapID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemapID);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        std::string resolvedPath = Path::resolve(faces[i]).string();
        unsigned char* data = stbi_load(resolvedPath.c_str(), &width, &height, &nrChannels, 3);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            Logger::error("Cubemap texture failed to load at path: " + faces[i]);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

bool SkyboxComponent::setFacePath(size_t index, const std::string& path) {
    if (index >= m_facePaths.size()) {
        return false;
    }

    m_facePaths[index] = Path::toAssetsRelative(path);
    loadCubemap(m_facePaths);
    return true;
}

bool SkyboxComponent::setFaces(const std::vector<std::string>& faces) {
    if (faces.size() != m_facePaths.size()) {
        return false;
    }

    m_facePaths = normalizeFaces(faces);
    loadCubemap(m_facePaths);
    return true;
}

void SkyboxComponent::setupMesh() {
    float skyboxVertices[] = { // Thanks learnopengl.com for these :)
        -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f
    };

    glGenVertexArrays(1, &m_skyboxVAO);
    glGenBuffers(1, &m_skyboxVBO);
    glBindVertexArray(m_skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

void SkyboxComponent::render(Renderer& renderer, const Camera& camera, const Transform& cameraTransform) {
    if (!isEnabled) return;
    if (m_cubemapID == 0) return;

    glDepthFunc(GL_GEQUAL);
    m_shader->use();

    m_shader->setVec3("colorTint", m_colorTint);

    glm::mat4 view = glm::mat4(glm::mat3(*(glm::mat4*)camera.getViewMatrix(cameraTransform)));
    if (m_rotation != 0.0f) {
        view = glm::rotate(view, glm::radians(m_rotation), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    
    Mat4 viewMat4;
    for (int col = 0; col < 4; col++)
        for (int row = 0; row < 4; row++)
            viewMat4[col][row] = view[col][row];
    m_shader->setMat4("u_View", viewMat4);

    Mat4 projMat4;
    camera.getProjectionMatrix(projMat4);
    m_shader->setMat4("u_Projection", projMat4);

    glBindVertexArray(m_skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemapID);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    
    glDepthFunc(GL_GREATER); 
}

void SkyboxComponent::serialize(nlohmann::json& j) const {
    j["type"] = "SkyboxComponent";
    j["faces"] = m_facePaths;
    j["rotation"] = m_rotation;
    j["colorTint"] = { m_colorTint.x, m_colorTint.y, m_colorTint.z };
}

void SkyboxComponent::deserialize(const nlohmann::json& j) {
    if (j.contains("rotation") && j["rotation"].is_number()) {
        m_rotation = j["rotation"].get<float>();
    }

    if (j.contains("faces") && j["faces"].is_array()) {
        m_facePaths = normalizeFaces(j["faces"].get<std::vector<std::string>>());
        if (m_facePaths.size() != 6) {
            m_facePaths = std::vector<std::string>(6);
        }
        loadCubemap(m_facePaths);
    }

    if (j.contains("colorTint") && j["colorTint"].is_array()) {
        auto tint = j["colorTint"].get<std::vector<float>>();
        if (tint.size() == 3) {
            m_colorTint = { tint[0], tint[1], tint[2] };
        }
    }
}
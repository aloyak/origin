#include "engine/render/model.h"
#include "engine/render/material.h"
#include "engine/render/mesh.h"
#include "engine/render/shader.h"
#include "engine/render/texture.h"
#include "engine/core/resourceManager.h"

#include "engine/lighting/directionalLight.h"
#include "engine/lighting/pointLight.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

#include "engine/utils/path.h"
#include "engine/utils/logger.h"

#include <cctype>

#ifdef __EMSCRIPTEN__
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif

static const aiTexture* findEmbeddedTexture(const aiScene* scene, const std::string& path) {
    if (const aiTexture* tex = scene->GetEmbeddedTexture(path.c_str())) {
        return tex;
    }

    auto normalize = [](std::string p) {
        size_t pos = p.find_last_of("\\/");
        if (pos != std::string::npos) p = p.substr(pos + 1);
        for (char& c : p) {
            if (c == '.') c = '_';
            c = std::tolower(static_cast<unsigned char>(c));
        }
        return p;
    };

    std::string target = normalize(path);

    for (unsigned int i = 0; i < scene->mNumTextures; i++) {
        std::string texName = scene->mTextures[i]->mFilename.C_Str();
        if (target == normalize(texName) && !target.empty()) {
            return scene->mTextures[i];
        }
    }

    return nullptr;
}

static std::string restoreExtension(std::string path) {
    const std::string extensions[] = { "_png", "_jpg", "_jpeg", "_tga", "_dds", "_bmp" };
    for (const auto& ext : extensions) {
        if (path.length() >= ext.length()) {
            if (path.compare(path.length() - ext.length(), ext.length(), ext) == 0) {
                path[path.length() - ext.length()] = '.';
                break;
            }
        }
    }
    return path;
}

Model::Model(const char* path, bool dynamic) : m_dynamic(dynamic) {
    loadModel(path);
}

void Model::draw(const Material& material,
                 const std::vector<DirectionalLight>& directionalLights,
                 const std::vector<PointLight>& pointLights) {
    for(unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].draw(material, directionalLights, pointLights);
}

void Model::drawInstanced(const Material& material,
                          const std::vector<DirectionalLight>& directionalLights,
                          const std::vector<PointLight>& pointLights,
                          unsigned int instanceVBO,
                          unsigned int instanceCount) {
    for (unsigned int i = 0; i < meshes.size(); i++) {
        meshes[i].drawInstanced(material, directionalLights, pointLights, instanceVBO, instanceCount);
    }
}

void Model::loadModel(std::string path) {
    path = Path::resolve(path).string();

    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        Logger::error(import.GetErrorString());
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));
    m_embeddedTextures.clear();

    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, const aiScene *scene) {
    for(unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    for(unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<MeshTexture> textures;

    for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.Position = Vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

        if (mesh->HasNormals()) {
            vertex.Normal = Vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        } else {
            vertex.Normal = Vec3(0.0f, 0.0f, 1.0f);
        }

        if(mesh->mTextureCoords[0]) {
            vertex.TexCoords = Vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        } else {
            vertex.TexCoords = Vec2(0.0f, 0.0f);
        }

        if (mesh->HasTangentsAndBitangents()) {
            vertex.Tangent = Vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
            vertex.Bitangent = Vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
        } else {
            vertex.Tangent = Vec3(1.0f, 0.0f, 0.0f);
            vertex.Bitangent = Vec3(0.0f, 1.0f, 0.0f);
        }
        vertices.push_back(vertex);
    }

    for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    std::vector<MeshTexture> diffuseMaps  = loadMaterialTextures(material, (int)aiTextureType_DIFFUSE,   "texture_diffuse",  scene);
    std::vector<MeshTexture> specularMaps = loadMaterialTextures(material, (int)aiTextureType_SPECULAR,  "texture_specular", scene);
    std::vector<MeshTexture> normalMaps   = loadMaterialTextures(material, (int)aiTextureType_NORMALS,   "texture_normal",   scene);

    if (normalMaps.empty()) {
        normalMaps = loadMaterialTextures(material, (int)aiTextureType_HEIGHT, "texture_normal", scene);
    }

    std::vector<MeshTexture> metallicMaps = loadMaterialTextures(material, (int)aiTextureType_METALNESS, "texture_metallic", scene);

    textures.insert(textures.end(), diffuseMaps.begin(),  diffuseMaps.end());
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    textures.insert(textures.end(), normalMaps.begin(),   normalMaps.end());
    textures.insert(textures.end(), metallicMaps.begin(), metallicMaps.end());

    Vec3 baseColor(1.0f, 1.0f, 1.0f);
    aiColor4D aiColor;
    if (aiGetMaterialColor(material, AI_MATKEY_BASE_COLOR, &aiColor) == AI_SUCCESS) {
        baseColor = Vec3(aiColor.r, aiColor.g, aiColor.b);
    } else if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &aiColor) == AI_SUCCESS) {
        baseColor = Vec3(aiColor.r, aiColor.g, aiColor.b);
    }

    return Mesh(vertices, indices, textures, m_dynamic, baseColor);
}

std::vector<MeshTexture> Model::loadMaterialTextures(
    aiMaterial *mat, int type, std::string typeName, const aiScene* scene)
{
    std::vector<MeshTexture> textures;
    aiTextureType assimpType = static_cast<aiTextureType>(type);

    for(unsigned int i = 0; i < mat->GetTextureCount(assimpType); i++) {
        aiString str;
        mat->GetTexture(assimpType, i, &str);
        const std::string texPath = str.C_Str();

        MeshTexture meshTex;
        meshTex.type = typeName;

        const aiTexture* aiTex = findEmbeddedTexture(scene, str.C_Str());

        if (aiTex) {
            auto it = m_embeddedTextures.find(texPath);
            if (it != m_embeddedTextures.end()) {
                meshTex.texture = it->second;
            } else {
                std::shared_ptr<Texture> tex;

                if (aiTex->mHeight == 0) {
                    tex = std::make_shared<Texture>(
                        reinterpret_cast<const unsigned char*>(aiTex->pcData),
                        aiTex->mWidth
                    );
                } else {
                    const int pixelCount = aiTex->mWidth * aiTex->mHeight;
                    std::vector<unsigned char> rgba(pixelCount * 4);
                    for (int p = 0; p < pixelCount; ++p) {
                        rgba[p*4+0] = aiTex->pcData[p].r;
                        rgba[p*4+1] = aiTex->pcData[p].g;
                        rgba[p*4+2] = aiTex->pcData[p].b;
                        rgba[p*4+3] = aiTex->pcData[p].a;
                    }
                    tex = std::make_shared<Texture>(rgba.data(), aiTex->mWidth, aiTex->mHeight, 4);
                }

                m_embeddedTextures[texPath] = tex;
                meshTex.texture = tex;
            }
        } else {
            std::string unmangledPath = restoreExtension(texPath);
            
            meshTex.texture = ResourceManager::instance().getTexture(directory + '/' + unmangledPath);
            
            if (!meshTex.texture && unmangledPath != texPath) {
                meshTex.texture = ResourceManager::instance().getTexture(directory + '/' + texPath);
            }
        }

        if (meshTex.texture) {
            textures.push_back(meshTex);
        }
    }
    return textures;
}
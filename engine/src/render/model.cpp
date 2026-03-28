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

#include "engine/utils/path.h"

#ifdef __EMSCRIPTEN__
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif

#include <spdlog/spdlog.h>

Model::Model(const char* path) {
    loadModel(path);
}

void Model::draw(const Material& material,
                 const std::vector<DirectionalLight>& directionalLights,
                 const std::vector<PointLight>& pointLights) {
    for(unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].draw(material, directionalLights, pointLights);
}

void Model::loadModel(std::string path) {
    path = Path::resolve(path).string();

    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        spdlog::error(import.GetErrorString());
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));

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

    std::vector<MeshTexture> diffuseMaps = loadMaterialTextures(material, (int)aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    std::vector<MeshTexture> specularMaps = loadMaterialTextures(material, (int)aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    std::vector<MeshTexture> normalMaps = loadMaterialTextures(material, (int)aiTextureType_NORMALS, "texture_normal");
    if (normalMaps.empty()) {
        normalMaps = loadMaterialTextures(material, (int)aiTextureType_HEIGHT, "texture_normal");
    }
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

    return Mesh(vertices, indices, textures);
}

std::vector<MeshTexture> Model::loadMaterialTextures(aiMaterial *mat, int type, std::string typeName) {
    std::vector<MeshTexture> textures;
    aiTextureType assimpType = static_cast<aiTextureType>(type);

    for(unsigned int i = 0; i < mat->GetTextureCount(assimpType); i++) {
        aiString str;
        mat->GetTexture(assimpType, i, &str);

        std::string fullPath = directory + '/' + str.C_Str();

        MeshTexture meshTex;
        meshTex.texture = ResourceManager::instance().getTexture(fullPath);
        meshTex.type = typeName;
        textures.push_back(meshTex);
    }
    return textures;
}
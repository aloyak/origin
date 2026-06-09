#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#include "engine/render/texture.h"
#include "engine/render/mesh.h"

class Shader;
class Material;
class DirectionalLight;
class PointLight;

struct aiNode;
struct aiMesh;
struct aiScene;
struct aiMaterial;

class Model {
public:
    Model(const char* path);
    void draw(const Material& material,
              const std::vector<DirectionalLight>& directionalLights,
              const std::vector<PointLight>& pointLights);

    const std::vector<Mesh>& getMeshes() const { return meshes; }

private:
    std::vector<Mesh> meshes;
    std::string directory;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_embeddedTextures;

    void loadModel(std::string path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);

    std::vector<MeshTexture> loadMaterialTextures(
        aiMaterial *mat, int type, std::string typeName, const aiScene* scene);
};
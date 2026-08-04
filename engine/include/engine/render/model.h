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

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<MeshTexture> textures;
    Vec3 baseColor;
};

class Model {
public:
    Model(bool dynamic = false);
    Model(const char* path, bool dynamic = false);
    
    void loadData(const char* path);
    void uploadToGPU();

    void draw(const Material& material,
              const std::vector<DirectionalLight>& directionalLights,
              const std::vector<PointLight>& pointLights);

    void drawInstanced(const Material& material,
                  const std::vector<DirectionalLight>& directionalLights,
                  const std::vector<PointLight>& pointLights,
                  unsigned int instanceVBO,
                  unsigned int instanceCount);

    const std::vector<Mesh>& getMeshes() const { return meshes; }
    std::vector<Mesh>& getMeshes() { return meshes; }

private:
    std::vector<Mesh> meshes;
    std::vector<MeshData> pendingMeshes;
    std::string directory;
    bool m_dynamic = false;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_embeddedTextures;

    void loadModel(std::string path);
    void processNode(aiNode *node, const aiScene *scene);
    MeshData processMesh(aiMesh *mesh, const aiScene *scene);

    std::vector<MeshTexture> loadMaterialTextures(
        aiMaterial *mat, int type, std::string typeName, const aiScene* scene);
};
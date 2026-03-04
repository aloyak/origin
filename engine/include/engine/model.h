#pragma once

#include <vector>
#include <string>
#include "engine/mesh.h"

class Shader;

struct aiNode;
struct aiMesh;
struct aiScene;
struct aiMaterial;

class Model {
public:
    Model(const char* path);
    void draw(Shader& shader);

private:
    std::vector<Mesh> meshes;
    std::string directory;

    void loadModel(std::string path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    
    std::vector<MeshTexture> loadMaterialTextures(aiMaterial *mat, int type, std::string typeName);
};
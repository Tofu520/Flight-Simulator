#ifndef GLTF_MODEL_H
#define GLTF_MODEL_H

#include "Mesh.h"
#include <vector>

//renders gltf models instead of the tinyobj basic model we were using before
class GltfModel {
public:
    explicit GltfModel(const char* path);
    ~GltfModel();

    void Draw(Shader& shader, Camera& camera);

private:
    std::vector<Mesh>   meshes;
    std::vector<GLuint> ownedTexIDs; //GL IDs we created; freed in destructor

    void load(const char* path);
};

#endif

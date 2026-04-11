#ifndef MODEL_CLASS_H
#define MODEL_CLASS_H

#include "tiny_obj_loader.h"
#include"Mesh.h"

class Model
{
public:
	Model(const char* file);

	void Draw(Shader& shader, Camera& camera);

private:
	const char* file;
	std::vector<Mesh> meshes;



	// Prevents textures from being loaded twice
	std::vector<std::string> loadedTexName;
	std::vector<Texture> loadedTex;

    void loadOBJ (const char * file);

	std::vector<Vertex> assembleVertices
	(
		std::vector<glm::vec3> positions, 
		std::vector<glm::vec3> normals, 
		std::vector<glm::vec2> texUVs
	);
    
};
#endif
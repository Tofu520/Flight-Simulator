#ifndef VBO_CLASS_H
#define VBO_CLASS_H

#include <glad/glad.h>
#include<glm/glm.hpp>
#include<vector>

// Structure to standardize the vertices used in the meshes instead of the GLfloat vertices we used before
struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 texUV;
};

class VBO
{
public:
	// Reference ID of the Vertex Buffer Object
	GLuint ID;
	// Constructor that generates a Vertex Buffer Object and links it to vertices
	VBO(std::vector<Vertex>& vertices);

	void Bind();

	void Unbind();
    
	void Delete();
};

#endif
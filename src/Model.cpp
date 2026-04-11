#define TINYOBJLOADER_IMPLEMENTATION
#include"Model.h"

Model::Model(const char* file)
{
	Model::file = file;
    loadOBJ(file);
}

void Model::Draw(Shader& shader, Camera& camera)
{
	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		meshes[i].Mesh::Draw(shader, camera);
	}

}

void Model::loadOBJ(const char* file)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    //load obj file
    std::string folder = "assets/Airplane/";
    folder = folder.substr(0, folder.find_last_of('/') + 1);
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, file, folder.c_str());

    if (!warn.empty())
        std::cout << "TinyOBJ warning: " << warn << std::endl;

    if (!err.empty())
        std::cerr << "TinyOBJ error: " << err << std::endl;

    if (!ret)
    {
        throw std::runtime_error("Failed to load OBJ file!");
    }

    for (const auto& shape : shapes)
    {
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;

        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
        {
            int fv = shape.mesh.num_face_vertices[f]; 

            for (size_t v = 0; v < fv; v++)
            {
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

                Vertex vertex;
                vertex.color = glm::vec3(1.0f, 1.0f, 1.0f);

                vertex.position = {
                    attrib.vertices[3 * idx.vertex_index + 0],
                    attrib.vertices[3 * idx.vertex_index + 1],
                    attrib.vertices[3 * idx.vertex_index + 2]
                };

                if (!attrib.normals.empty())
                {
                    vertex.normal = {
                        attrib.normals[3 * idx.normal_index + 0],
                        attrib.normals[3 * idx.normal_index + 1],
                        attrib.normals[3 * idx.normal_index + 2]
                    };
                }
                else
                {
                    vertex.normal = {0.0f, 0.0f, 0.0f};
                }

                if (!attrib.texcoords.empty())
                {
                    vertex.texUV = {
                        attrib.texcoords[2 * idx.texcoord_index + 0],
                        attrib.texcoords[2 * idx.texcoord_index + 1]
                    };
                }
                else
                {
                    vertex.texUV = {0.0f, 0.0f};
                }

                vertices.push_back(vertex);
                indices.push_back(static_cast<GLuint>(indices.size()));
            }

            index_offset += fv;
        }

        std::vector<Texture> textures;
        for (auto &mat : materials) {
            if (!mat.diffuse_texname.empty()) {
                textures.push_back(Texture(("assets/Airplane/" + mat.diffuse_texname).c_str(), "diffuse", textures.size(), GL_RGB, GL_UNSIGNED_BYTE));
            }
        }
        meshes.push_back(Mesh(vertices, indices, textures));

    }
}

std::vector<Vertex> Model::assembleVertices
(
	std::vector<glm::vec3> positions,
	std::vector<glm::vec3> normals,
	std::vector<glm::vec2> texUVs
)
{
	std::vector<Vertex> vertices;
	for (int i = 0; i < positions.size(); i++)
	{
		vertices.push_back
		(
			Vertex
			{
				positions[i],
				normals[i],
				glm::vec3(1.0f, 1.0f, 1.0f),
				texUVs[i]
			}
		);
	}
	return vertices;
}
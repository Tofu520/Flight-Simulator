#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "GltfModel.h"
#include <stb/stb_image.h>
#include <glad/glad.h>
#include <iostream>
#include <stdexcept>
#include <string>

//Upload raw image bytes (PNG/JPG embedded in GLB) to a new GL texture ID
//Returns 0 on failure
static GLuint uploadEmbeddedTexture(const unsigned char* mem, int len)
{
    int w, h, ch;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* px = stbi_load_from_memory(mem, len, &w, &h, &ch, 0);
    if (!px) {
        std::cerr << "GltfModel: stbi failed on embedded image\n";
        return 0;
    }

    GLuint id;
    glGenTextures(1, &id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);
    GLenum fmt = (ch == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, px);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(px);
    return id;
}

GltfModel::GltfModel(const char* path) { load(path); }

GltfModel::~GltfModel()
{
    if (!ownedTexIDs.empty())
        glDeleteTextures((GLsizei)ownedTexIDs.size(), ownedTexIDs.data());
}

void GltfModel::Draw(Shader& shader, Camera& camera)
{
    for (auto& m : meshes) m.Draw(shader, camera);
}

void GltfModel::load(const char* path)
{
    cgltf_options opts = {};
    cgltf_data*   data = nullptr;

    if (cgltf_parse_file(&opts, path, &data) != cgltf_result_success)
        throw std::runtime_error(std::string("GltfModel: failed to parse ") + path);

    if (cgltf_load_buffers(&opts, data, path) != cgltf_result_success) {
        cgltf_free(data);
        throw std::runtime_error("GltfModel: failed to load buffers");
    }

    //Upload every embedded image once; index mirrors data->images[]
    std::vector<GLuint> imgTex(data->images_count, 0);
    for (cgltf_size i = 0; i < data->images_count; i++) {
        cgltf_image& img = data->images[i];
        if (img.buffer_view && img.buffer_view->buffer->data) {
            const auto* mem =
                (const unsigned char*)img.buffer_view->buffer->data
                + img.buffer_view->offset;
            imgTex[i] = uploadEmbeddedTexture(mem, (int)img.buffer_view->size);
            if (imgTex[i]) ownedTexIDs.push_back(imgTex[i]);
        }
    }

    //Build Mesh for each primitive
    for (cgltf_size mi = 0; mi < data->meshes_count; mi++) {
        cgltf_mesh& cmesh = data->meshes[mi];
        for (cgltf_size pi = 0; pi < cmesh.primitives_count; pi++) {
            cgltf_primitive& prim = cmesh.primitives[pi];
            if (prim.type != cgltf_primitive_type_triangles) continue;

            const cgltf_accessor* posAcc  = nullptr;
            const cgltf_accessor* normAcc = nullptr;
            const cgltf_accessor* uvAcc   = nullptr;

            for (cgltf_size a = 0; a < prim.attributes_count; a++) {
                switch (prim.attributes[a].type) {
                    case cgltf_attribute_type_position:
                        posAcc  = prim.attributes[a].data; break;
                    case cgltf_attribute_type_normal:
                        normAcc = prim.attributes[a].data; break;
                    case cgltf_attribute_type_texcoord:
                        if (!uvAcc) uvAcc = prim.attributes[a].data; break;
                    default: break;
                }
            }
            if (!posAcc) continue;

            cgltf_size vcount = posAcc->count;
            std::vector<Vertex> vertices(vcount);
            float f3[3] = {}, f2[2] = {};

            for (cgltf_size v = 0; v < vcount; v++) {
                cgltf_accessor_read_float(posAcc, v, f3, 3);
                vertices[v].position = {f3[0], f3[1], f3[2]};

                if (normAcc) {
                    cgltf_accessor_read_float(normAcc, v, f3, 3);
                    vertices[v].normal = {f3[0], f3[1], f3[2]};
                } else {
                    vertices[v].normal = {0.0f, 1.0f, 0.0f};
                }

                if (uvAcc) {
                    cgltf_accessor_read_float(uvAcc, v, f2, 2);
                    vertices[v].texUV = {f2[0], f2[1]};
                }

                vertices[v].color = {1.0f, 1.0f, 1.0f};
            }

            std::vector<GLuint> indices;
            if (prim.indices) {
                cgltf_size icount = prim.indices->count;
                indices.resize(icount);
                for (cgltf_size i = 0; i < icount; i++) {
                    cgltf_uint val = 0;
                    cgltf_accessor_read_uint(prim.indices, i, &val, 1);
                    indices[i] = (GLuint)val;
                }
            } else {
                indices.resize(vcount);
                for (GLuint i = 0; i < (GLuint)vcount; i++) indices[i] = i;
            }

            std::vector<Texture> textures;
            if (prim.material && prim.material->has_pbr_metallic_roughness) {
                auto& pbr = prim.material->pbr_metallic_roughness;
                if (pbr.base_color_texture.texture &&
                    pbr.base_color_texture.texture->image) {
                    cgltf_image* img = pbr.base_color_texture.texture->image;
                    int idx = (int)(img - data->images);
                    if (idx >= 0 && idx < (int)imgTex.size() && imgTex[idx]) {
                        textures.emplace_back(imgTex[idx], "diffuse", 0u);
                    }
                }
            }

            meshes.emplace_back(vertices, indices, textures);
        }
    }

    std::cout << "GltfModel: loaded " << meshes.size()
              << " primitives from " << path << "\n";
    cgltf_free(data);
}

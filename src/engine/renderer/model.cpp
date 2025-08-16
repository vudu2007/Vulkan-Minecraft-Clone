#include "model.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

Model::Model(const std::string model_file_path, const float scale)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, model_file_path.c_str()))
    {
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<Vertex, uint32_t> unique_vertices{};
    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex{};
            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0] * scale,
                attrib.vertices[3 * index.vertex_index + 1] * scale,
                attrib.vertices[3 * index.vertex_index + 2] * scale,
            };
            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
            };
            vertex.color = {
                1.0f,
                1.0f,
                1.0f,
            };
            vertex.normal = {
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2],
            };

            if (unique_vertices.count(vertex) == 0)
            {
                unique_vertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }
            indices.push_back(unique_vertices[vertex]);
        }
    }
}

Model::Model(const std::vector<Vertex>& vertices, const std::vector<Index>& indices)
    : vertices(vertices), indices(indices)
{
}

const std::vector<Model::Vertex>& Model::getVertices() const
{
    return vertices;
}

const std::vector<Model::Index>& Model::getIndices() const
{
    return indices;
}

void Model::translate(const glm::vec3 units)
{
    for (auto& vertex : vertices)
    {
        vertex.pos += units;
    }
}

Model::Vertex::Vertex(const glm::vec3& pos, const glm::vec3& normal, const glm::vec3& color, const glm::vec2& tex_coord)
    : pos(pos), normal(normal), color(color), texCoord(tex_coord)
{
}

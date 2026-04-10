#pragma once

#include "device.hpp"

#include "../usage/glm-usage.hpp"
#include <glm/gtx/hash.hpp>

#include <array>
#include <string>

class Model
{
  public:
    struct Vertex
    {
        static const uint32_t BINDING = 0;

        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec3 color;
        glm::vec2 texCoord;

        explicit Vertex(
            const glm::vec3& pos,
            const glm::vec3& normal,
            const glm::vec3& color,
            const glm::vec2& tex_coord);

        bool operator==(const Vertex& other) const
        {
            return (pos == other.pos) && (normal == other.normal) && (color == other.color) &&
                   (texCoord == other.texCoord);
        }

        static VkVertexInputBindingDescription getBindingDescription()
        {
            return {
                .binding = BINDING,
                .stride = sizeof(Vertex),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            };
        }

        static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions()
        {
            return {{
                {
                    .location = 0,
                    .binding = BINDING,
                    .format = VK_FORMAT_R32G32B32_SFLOAT,
                    .offset = offsetof(Vertex, pos),
                },
                {
                    .location = 1,
                    .binding = BINDING,
                    .format = VK_FORMAT_R32G32B32_SFLOAT,
                    .offset = offsetof(Vertex, normal),
                },
                {
                    .location = 2,
                    .binding = BINDING,
                    .format = VK_FORMAT_R32G32B32_SFLOAT,
                    .offset = offsetof(Vertex, color),
                },
                {
                    .location = 3,
                    .binding = BINDING,
                    .format = VK_FORMAT_R32G32_SFLOAT,
                    .offset = offsetof(Vertex, texCoord),
                },
            }};
        }
    };

    struct InstanceData
    {
        static const uint32_t BINDING = 1;

        glm::vec3 pos;

        static VkVertexInputBindingDescription getBindingDescription()
        {
            return {
                .binding = 1,
                .stride = sizeof(InstanceData),
                .inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
            };
        }

        static std::array<VkVertexInputAttributeDescription, 1> getAttributeDescriptions()
        {
            return {{
                {
                    .location = 4,
                    .binding = BINDING,
                    .format = VK_FORMAT_R32G32B32_SFLOAT,
                    .offset = offsetof(InstanceData, pos),
                },
            }};
        }
    };

    struct UniformBufferObject
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };

    using Index = uint32_t;

  public:
    Model() = default;
    Model(const std::string& model_file_path, float scale = 1.0f);
    Model(const std::vector<Vertex>& vertices, const std::vector<Index>& indices);

    const std::vector<Vertex>& getVertices() const;
    const std::vector<Index>& getIndices() const;

    void translate(const glm::vec3& units);

  private:
    std::vector<Vertex> vertices;
    std::vector<Index> indices;
    std::vector<glm::vec3> normals;
};

namespace std
{

template <>
struct hash<Model::Vertex>
{
    size_t operator()(Model::Vertex const& vertex) const
    {
        const std::size_t h_pos = hash<glm::vec3>{}(vertex.pos);
        const std::size_t h_normal = hash<glm::vec3>{}(vertex.normal);
        const std::size_t h_color = hash<glm::vec3>{}(vertex.color);
        const std::size_t h_tex_coord = hash<glm::vec2>{}(vertex.texCoord);
        return ((((h_pos ^ (h_normal << 1)) >> 1) ^ (h_color << 1)) >> 1) ^ (h_tex_coord << 1);
    }
};

} // namespace std

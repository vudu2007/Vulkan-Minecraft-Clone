#ifndef VMC_SRC_ENGINE_RENDERER_MODEL_HPP
#define VMC_SRC_ENGINE_RENDERER_MODEL_HPP

#include "device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <GLM/glm.hpp>
#include <GLM/gtx/hash.hpp>

#include <array>

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

        bool operator==(const Vertex& other) const
        {
            return pos == other.pos && normal == other.normal && color == other.color && texCoord == other.texCoord;
        }

        static VkVertexInputBindingDescription getBindingDescription()
        {
            VkVertexInputBindingDescription binding_description{};
            binding_description.binding = BINDING;
            binding_description.stride = sizeof(Vertex);
            binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return binding_description;
        }

        static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions()
        {
            std::array<VkVertexInputAttributeDescription, 4> attribute_descriptions{};

            attribute_descriptions[0].binding = BINDING;
            attribute_descriptions[0].location = 0;
            attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_descriptions[0].offset = offsetof(Vertex, pos);

            attribute_descriptions[1].binding = BINDING;
            attribute_descriptions[1].location = 1;
            attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_descriptions[1].offset = offsetof(Vertex, normal);

            attribute_descriptions[2].binding = BINDING;
            attribute_descriptions[2].location = 2;
            attribute_descriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_descriptions[2].offset = offsetof(Vertex, color);

            attribute_descriptions[3].binding = BINDING;
            attribute_descriptions[3].location = 3;
            attribute_descriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
            attribute_descriptions[3].offset = offsetof(Vertex, texCoord);

            return attribute_descriptions;
        }
    };

    struct InstanceData
    {
        static const uint32_t BINDING = 1;

        glm::vec3 pos;

        static VkVertexInputBindingDescription getBindingDescription()
        {
            VkVertexInputBindingDescription binding_description{};
            binding_description.binding = 1;
            binding_description.stride = sizeof(InstanceData);
            binding_description.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

            return binding_description;
        }

        static std::array<VkVertexInputAttributeDescription, 1> getAttributeDescriptions()
        {
            std::array<VkVertexInputAttributeDescription, 1> attribute_descriptions{};

            attribute_descriptions[0].binding = BINDING;
            attribute_descriptions[0].location = 4;
            attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_descriptions[0].offset = offsetof(InstanceData, pos);

            return attribute_descriptions;
        }
    };

    struct UniformBufferObject
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };

    using Index = uint32_t;

  private:
    std::vector<Vertex> vertices;
    std::vector<Index> indices;
    std::vector<glm::vec3> normals;

  public:
    Model(const std::string model_file_path, const float scale = 1.0f);
    Model(const std::vector<Vertex>& vertices, const std::vector<Index>& indices);

    const std::vector<Vertex>& getVertices() const;
    const std::vector<Index>& getIndices() const;

    void translate(const glm::vec3 units);
};

namespace std
{

template <> struct hash<Model::Vertex>
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

#endif // VMC_SRC_ENGINE_RENDERER_MODEL_HPP

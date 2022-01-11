#include "Geometry/OptiCloudVertex.h"

VkVertexInputBindingDescription OptiCloudVertex::GetBindingDescription()
{
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(OptiCloudVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}
std::vector<VkVertexInputAttributeDescription> OptiCloudVertex::GetAttributeDescriptions()
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(OptiCloudVertex, Pos);
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32_UINT;
    attributeDescriptions[1].offset = offsetof(OptiCloudVertex, Color);
    return attributeDescriptions;
}
#pragma once
#include <glm/gtc/type_precision.hpp>
#include <glm/vec3.hpp>
#include <vulkan/vulkan.h>
#include <vector>

/// @brief
///  A optimize cloud vertex that is 16 bytes.
struct OptiCloudVertex
{
    /// Position (12 bytes).
    glm::vec3 Pos{};
    /// Color (3 bytes).
    glm::u8vec3 Color{};
    /// Attribute (1 byte) - not used.
    uint8_t Attribute{};

    static VkVertexInputBindingDescription GetBindingDescription();
    static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
};
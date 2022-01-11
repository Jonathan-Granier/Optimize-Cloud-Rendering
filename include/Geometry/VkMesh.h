#pragma once

#include "Geometry/Mesh.h"
#include "Olympus/Device.h"
#include "Olympus/MemoryBuffer.h"

/// @brief
///  Class which holds, allocates and draws a mesh.
class VkMesh
{
public:
    VkMesh(const olp::Device &iDevice);
    ~VkMesh() = default;
    VkMesh(const VkMesh &) = delete;
    VkMesh(VkMesh &&ioCloud) noexcept = default;
    VkMesh &operator=(const VkMesh &) = delete;
    VkMesh &operator=(VkMesh &&ioCloud) noexcept = default;

    void InitCube();
    void InitQuad();
    void Destroy();

    VkBuffer GetVertexBuffer() const { return m_VertexBuffer.Buffer; }
    VkBuffer GetIndexBuffer() const { return m_IndexBuffer.Buffer; }
    uint32_t GetIndexCount() const { return static_cast<uint32_t>(m_Mesh.Indices.size() * 3); }

    void Draw(VkCommandBuffer commandBuffer);

private:
    ///  Allocate the mesh vertices in the gpu memory.
    void CreateVertexBuffer();

    ///  Allocate the mesh indices (triangle) in the gpu memory.
    void CreateIndexBuffer();

    const olp::Device &m_Device;
    Mesh m_Mesh;

    olp::MemoryBuffer m_VertexBuffer;
    olp::MemoryBuffer m_IndexBuffer;
};
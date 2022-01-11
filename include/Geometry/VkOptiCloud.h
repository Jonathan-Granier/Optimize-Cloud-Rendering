#pragma once

#include "Olympus/Device.h"
#include "Olympus/CommandBuffer.h"
#include "Olympus/MemoryBuffer.h"
#include "Geometry/OptiCloudVertex.h"

///  Class which holds, allocates and draws a optimize cloud.
class VkOptiCloud
{
public:
    VkOptiCloud(const olp::Device &iDevice);
    ~VkOptiCloud();

    VkOptiCloud(const VkOptiCloud &) = delete;
    VkOptiCloud(VkOptiCloud &&ioCloud) noexcept = default;
    VkOptiCloud &operator=(const VkOptiCloud &) = delete;
    VkOptiCloud &operator=(VkOptiCloud &&ioCloud) noexcept = default;

    olp::MemoryBuffer &GetVertexBuffer() { return m_VertexBuffer; }
    olp::MemoryBuffer &GetReprojectedBuffer() { return m_ReprojectedBuffer; }

    uint32_t GetVertexBufferSize() { return m_VertexBufferSize; }
    uint32_t GetReprojectedBufferSize() { return m_ReprojectedBufferSize; }

    void SetPointsByStep(uint32_t iPointCount) { m_NbPointByStep = iPointCount; }

    void Init();

    void CreateReprojectedBuffer(uint32_t iWidth, uint32_t iHeight);
    void DestroyReprojectedBuffer();

    void Destroy();

    ///  Draw NbPointByStep of the VertexBuffer and increment the step.
    /// @param[in] iCommandBuffer Current command buffer.
    void DrawVertexBuffer(VkCommandBuffer iCommandBuffer);

    ///  Draw the reprojected buffer.
    /// @param[in] iCommandBuffer Current command buffer.
    void DrawReprojectedBuffer(VkCommandBuffer iCommandBuffer);

    ///  Reset the current step to 0. To use when the scene is moving.
    void ResetDraw();

protected:
    void CreateVertexBuffer(const std::vector<OptiCloudVertex> &iPoints);

    /// Number of vertex in the cloud.
    uint32_t m_NbVertex = 0;
    /// Size of the vertex buffer. nbVertex * sizeof(OptiCloudVertex).
    uint32_t m_VertexBufferSize = 0;
    /// Number of points to draw at each step. Convergence speed.
    uint32_t m_NbPointByStep = 100'000;
    /// Size of the reprojected buffer. Surface size * sizeof(CloudVertex).
    uint32_t m_ReprojectedBufferSize = 0;
    /// Number of vertex in the reprojected buffer.
    uint32_t m_NbReprojectedVertex = 0;

    /// Current step. Used by DrawVertexBuffer.
    uint32_t m_Step = 0;

    const olp::Device &m_Device;
    /// A OptiCloudVertex buffer the size of the cloud.
    olp::MemoryBuffer m_VertexBuffer;
    /// A CloudVertex buffer the size of the surface.
    olp::MemoryBuffer m_ReprojectedBuffer;
};
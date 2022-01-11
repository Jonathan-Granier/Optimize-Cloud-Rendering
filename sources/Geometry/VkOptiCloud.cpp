#include "Geometry/VkOptiCloud.h"
#include "Geometry/CloudVertex.h"
#include "Prime.h"
#include "Olympus/CommandBuffer.h"
#include <iostream>
#include <random>

VkOptiCloud::VkOptiCloud(const olp::Device &iDevice)
    : m_Device(iDevice)
{
}

//----------------------------------------------------------------------------------------------------------------------
VkOptiCloud::~VkOptiCloud()
{
    Destroy();
}

//----------------------------------------------------------------------------------------------------------------------
void VkOptiCloud::Init()
{
    m_NbVertex = 5'000'000;
    m_VertexBufferSize = m_NbVertex * sizeof(OptiCloudVertex);

    std::vector<OptiCloudVertex> points(m_NbVertex);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-10.0, 10.0);

    for (uint32_t i = 0; i < m_NbVertex; ++i)
    {
        points[i].Pos = {dis(gen), dis(gen), 0.0f};
        points[i].Color = {0, 255, 255};
    }

    std::cout << "Create opti cloud with " << points.size() << " points. m_NbVertex : " << m_NbVertex
              << " m_BufferSize : " << m_VertexBufferSize << std::endl;

    CreateVertexBuffer(points);
    ResetDraw();
}

//----------------------------------------------------------------------------------------------------------------------
void VkOptiCloud::DestroyReprojectedBuffer()
{
    m_ReprojectedBuffer.Destroy();
}

//----------------------------------------------------------------------------------------------------------------------
void VkOptiCloud::Destroy()
{
    m_VertexBuffer.Destroy();
    DestroyReprojectedBuffer();
}

//----------------------------------------------------------------------------------------------------------------------
void VkOptiCloud::DrawVertexBuffer(VkCommandBuffer iCommandBuffer)
{
    if (m_Step * m_NbPointByStep >= m_NbVertex)
        return;

    uint32_t drawPointCount = m_NbVertex - m_Step * m_NbPointByStep;
    drawPointCount = std::min(drawPointCount, m_NbPointByStep);

    VkBuffer vertexBuffers[] = {m_VertexBuffer.Buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(iCommandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdDraw(
        iCommandBuffer, drawPointCount, 1, m_Step * m_NbPointByStep, 0);
    m_Step++;
}

//----------------------------------------------------------------------------------------------------------------------
void VkOptiCloud::DrawReprojectedBuffer(VkCommandBuffer iCommandBuffer)
{
    VkBuffer vertexBuffers[] = {m_ReprojectedBuffer.Buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(iCommandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdDraw(iCommandBuffer, m_NbReprojectedVertex, 1, 0, 0);
}

//----------------------------------------------------------------------------------------------------------------------
void VkOptiCloud::CreateVertexBuffer(const std::vector<OptiCloudVertex> &iPoints)
{
    olp::MemoryBuffer stagingBuffer = m_Device.CreateMemoryBuffer(
        m_VertexBufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    stagingBuffer.TransferDataInBuffer(iPoints, m_VertexBufferSize);

    m_VertexBuffer = m_Device.CreateMemoryBuffer(
        m_VertexBufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_VertexBuffer.CopyFrom(stagingBuffer.Buffer, m_VertexBufferSize);
    stagingBuffer.Destroy();
}

//----------------------------------------------------------------------------------------------------------------------
void VkOptiCloud::CreateReprojectedBuffer(uint32_t iWidth, uint32_t iHeight)
{
    m_NbReprojectedVertex = iWidth * iHeight;
    m_ReprojectedBufferSize = m_NbReprojectedVertex * sizeof(CloudVertex);
    m_ReprojectedBuffer = m_Device.CreateMemoryBuffer(
        m_ReprojectedBufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}
//----------------------------------------------------------------------------------------------------------------------
void VkOptiCloud::ResetDraw()
{
    m_Step = 0;
}
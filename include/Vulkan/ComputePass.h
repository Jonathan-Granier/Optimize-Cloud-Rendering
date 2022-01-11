#pragma once
#include "Geometry/VkOptiCloud.h"
#include "Olympus/PipelineLayout.h"
#include "Olympus/DescriptorSet.h"
#include "Olympus/Device.h"
#include "Olympus/MemoryBuffer.h"

///  Compute pass for the optimize cloud rendering.
///
/// Use the vertex buffer and the Vertex index image to fill the reprojected buffer.
class ComputePass
{
public:
    ///  Constructor.
    /// @param[in] iDevice Device to initialize the compute pass with.
    explicit ComputePass(const olp::Device &device);

    ///  Destructor.
    void Destroy();

    ///  Creates the compute pass.
    /// @param[in] iDescriptorPool Descriptor pool to allocate descriptor of the pass.
    /// @param[in] iOptiCloud Optimize cloud.
    /// @param[in] iVertexIndexImageView Vertex index image filled by the graphic pass.
    /// @param[in] iScreenSize Uniform buffer of the screen size.
    /// @param[in] iWidth VertexIndexImage width.
    /// @param[in] iHeight VertexIndexImage height.
    void Create(
        VkDescriptorPool &iDescriptorPool,
        VkOptiCloud &iOptiCloud,
        VkImageView iVertexIndexImageView,
        olp::UniformBuffer &iScreenSize,
        uint32_t iWidth,
        uint32_t iHeight);

    ///  Submits the command buffer to the compute queue.
    /// @param[in] iWaitSemaphore Semaphore to wait before execute the pass.
    /// @param[in] iSignalSemaphore Semaphore to signal when the execution is finished.
    void Process(VkSemaphore iWaitSemaphore, VkSemaphore iSignalSemaphore);

    /// Wait the fence of the compute pass.
    void WaitFence();

    VkSemaphore GetSemaphore() { return m_Semaphore; }
    VkCommandBuffer GetCommandBuffer() { return m_CommandBuffer; }

protected:
    ///  Create the pipeline layout.
    void CreatePipelineLayout();

    ///  Create the descriptors.
    /// @param[in] iDescriptorPool Descriptor pool to allocate descriptor of the pass.
    /// @param[in] iOptiCloud Optimize cloud.
    /// @param[in] iVertexIndexImageView Vertex index image filled by the graphic pass.
    /// @param[in] iScreenSize Uniform buffer of the screen size.
    void CreateDescriptor(
        VkDescriptorPool &iDescriptorPool,
        VkOptiCloud &iOptiCloud,
        VkImageView iVertexIndexImageView,
        olp::UniformBuffer &iScreenSize);

    ///  Create the pipeline.
    void CreatePipeline();

    ///  Create the command pool and the command buffer.
    void CreateCommandPoolAndBuffer();

    /// Create the sempahore and the fence used for the sync.
    void CreateSemaphore();

    ///  Build the command buffer.
    /// @param[in] iWidth VertexIndexImage width.
    /// @param[in] iHeight VertexIndexImage height.
    void BuildCommandBuffer(uint32_t iWidth, uint32_t iHeight);

    /// Vulkan device.
    const olp::Device &m_Device;

    /// Command pool for the compute queue.
    VkCommandPool m_CommandPool;
    /// Command buffer storing the dispatch commands and barriers.
    VkCommandBuffer m_CommandBuffer;
    /// Execution dependency between compute & graphic submission.
    VkSemaphore m_Semaphore;
    /// Synchronisation GPU/CPU. Need to find a better solution.
    VkFence m_Fence;

    /// Layout of the compute pipeline.
    olp::PipelineLayout m_PipelineLayout;
    /// Descriptor of the compute pass.
    olp::DescriptorSet m_DescriptorSet;
    /// Compute pipeline.
    VkPipeline m_Pipeline;
};
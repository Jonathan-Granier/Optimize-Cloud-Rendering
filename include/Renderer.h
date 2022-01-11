#pragma once

#include "Vulkan/ComputePass.h"
#include "Geometry/OptiCloudVertex.h"
#include "Geometry/VkMesh.h"
#include "Geometry/VkCloud.h"
#include "Geometry/VkOptiCloud.h"
#include "Olympus/Instance.h"
#include "Olympus/CloudPipeline.h"
#include "Olympus/MeshPipeline.h"
#include "Olympus/Pipeline.h"
#include "Olympus/PipelineLayout.h"
#include "Olympus/CommandBuffer.h"
#include "Olympus/DescriptorSet.h"
#include "Olympus/Device.h"
#include "Olympus/Shader.h"
#include "Olympus/Swapchain.h"
#include "Olympus/Texture.h"
#include "Olympus/UniformBuffer.h"
#include <glm/glm.hpp>
#include <memory>

class Camera;

struct ModelInfo
{
    glm::mat4 ModelMat;
    glm::mat4 MVPMat;
};

struct CameraInfo
{
    glm::mat4 ViewMat;
    glm::mat4 InvViewMat;
    glm::mat4 ProjMat;
    glm::mat4 InvProjMat;
    glm::vec3 CamPos;
};

struct ScreenSize
{
    uint32_t Width;
    uint32_t Height;
};

struct Lighting
{
    glm::vec3 Color;
    float Intensity;
    bool ThreePointEnabled;
};

struct PointSize
{
    uint32_t Size;
};

struct UniformBuffers
{
    olp::UniformBuffer Model;
    olp::UniformBuffer Camera;
    olp::UniformBuffer ScreenSize;
    olp::UniformBuffer Lighting;
    olp::UniformBuffer PointSize;
};

/// @brief
///  Renderer class, handling all necessary Vulkan operations.
class Renderer
{
public:
    ///  Constructs the renderer, creating the necessary objects & initializing resources.
    /// @param[in] iInstance Vulkan instance to initialize the device with.
    /// @param[in] iSurface Vulkan surface to initialize the device with.
    /// @param[in] iWidth Swapchain width.
    /// @param[in] iHeight Swapchain height.
    Renderer(const olp::Instance &iInstance, VkSurfaceKHR iSurface, uint32_t iWidth, uint32_t iHeight);

    ///  Initializes Vulkan resources.
    void InitResources();

    /// @brief
    ///  Releases Vulkan resources.
    void ReleaseResources();

    /// @brief
    ///  Recreates swapchain resources.
    /// @param iWidth New swapchain width.
    /// @param iHeight New swapchain height.
    void RecreateSwapchainResources(uint32_t iWidth, uint32_t iHeight);

    /// @brief
    ///  Releases swapchain resources.
    void ReleaseSwapchainResources();

    /// @brief
    ///  Recreates all pipelines.
    void RecreatePipelines();

    /// @brief
    ///  Imports & adds a cloud to be rendered.
    /// @param iFilePath Path to the mesh to be imported.
    void AddCloud(const std::filesystem::path &iFilePath);

    /// @brief
    ///  Enables [3-point lighting](https://en.wikipedia.org/wiki/Three-point_lighting).
    /// @param iEnabled True to enable, false to disable.
    void Enable3PointLighting(bool iEnabled = true);

    /// @brief
    ///  Sets the light color.
    /// @param iLightColor New light color.
    void SetLightColor(const glm::vec3 &iLightColor);

    /// @brief
    ///  Sets the light intensity.
    /// @param iIntensity New light intensity.
    void SetLightIntensity(float iIntensity);

    /// @brief
    ///  Sets the point size.
    /// @param iPointSize New point size.
    void UpdatePointSize(uint32_t iPointSize);

    /// @brief
    ///  Sets the number of points to be loaded each step.
    /// @param iPointCount New points by step count.
    void UpdatePointsByStep(uint32_t iPointCount);

    ///  Renders the next frame.
    void DrawNextFrame(const glm::mat4 &iView, const glm::mat4 &iProj);

protected:
    ///  Creates swapchain resources (pipelines, framebuffers, descriptors, ...).
    void CreateSwapchainRessources();

    /// @brief
    ///  Initializes the default scene's geometry.
    void InitGeometry();

    /// @brief
    ///  Creates the pipeline layout.
    void CreatePipelineLayout();

    /// @brief
    ///  Creates the main render pass.
    void CreateRenderPass();

    /// @brief
    ///  Creates the uniform buffers.
    void CreateUniformBuffers();

    /// @brief
    ///  Creates the descriptor pool.
    void CreateDescriptorPool();

    /// @brief
    ///  Creates the descriptor sets.
    void CreateDescriptorSets();

    /// @brief
    ///  Create the command buffers.
    void CreateCommandBuffers();

    /// @brief
    ///  Creates the synchronization objets.
    void CreateSyncObjects();

    /// @brief
    ///  Creates the depth buffer.
    void CreateDepthBuffer();

    /// @brief
    ///  Creates the vertex index image.
    void CreateVertexIndexImage();

    /// @brief
    ///  Creates the pipelines.
    void CreatePipelines();

    ///  Updates the camera's uniform buffers.
    void UpdateUniformBuffers(const glm::mat4 &iView, const glm::mat4 &iProj);

    /// @brief
    ///  Builds the command buffer at the given index.
    /// @param iIndex Index of the command buffer to build.
    void BuildCommandBuffer(uint32_t iIndex);

    /// @brief
    ///  Finds the appropriate depth format.
    /// @return Chosen depth format.
    VkFormat FindDepthFormat();

    /// Vulkan device that contains instance, physical device, device and queue.
    olp::Device m_Device;
    /// Swapchain.
    olp::Swapchain m_Swapchain;

    /// Descriptor of the main render pass (cloud , mesh and optimize cloud).
    olp::DescriptorSet m_MainPassDescriptor;
    /// Descriptor of the gradient pass.
    olp::DescriptorSet m_GradientPassDescriptor;
    /// Descriptor pool.
    VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;

    /// Pipeline layout of the main render pass.
    olp::PipelineLayout m_PipelineLayout;
    /// Pipeline layout for the gradient pass.
    olp::PipelineLayout m_GradientPipelineLayout;
    /// Pipeline for the gradient pass. (Render a quad , so need a mesh pipeline).
    olp::MeshPipeline<MeshVertex> m_GradientPipeline;
    /// Optimize cloud pipeline.
    olp::CloudPipeline<OptiCloudVertex> m_OptiCloudPipeline;
    /// Reprojected cloud pipeline.
    olp::CloudPipeline<CloudVertex> m_ReprojectedPipeline;
    /// Cloud pipeline.
    olp::CloudPipeline<CloudVertex> m_CloudPipeline;
    /// Mesh pipeline.
    olp::MeshPipeline<MeshVertex> m_MeshPipeline;

    /// Graphics render pass.
    VkRenderPass m_RenderPass = VK_NULL_HANDLE;

    /// Command buffer for the graphics queue. ( 1 by framebuffer of the swapchain).
    std::vector<olp::CommandBuffer> m_CommandBuffers{};

    /// Compute pass for the optimize cloud rendering.
    ComputePass m_PreparePass;

    /// Maximum number of frames to calculate in parallel.
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    /// Semaphore to know if the current image is available. Already presented by the swapchain.
    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_ImageAvailableSemaphores{};
    /// Semaphore to know if the rendering is finished for current image.
    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_RenderFinishedSemaphores{};
    /// Fence to synchronize GPU/CPU for update uniform buffer.
    std::array<VkFence, MAX_FRAMES_IN_FLIGHT> m_InFlightFences{};
    std::vector<VkFence> m_ImagesInFlight{};
    /// Current frame index in the swapchain
    size_t m_CurrentFrame = 0;

    /// Depth buffer image.
    olp::Image m_DepthBuffer;
    /// Image to hold the indices of the rendered optimize cloud vertex.
    olp::Image m_VertexIndexImage;

    /// A set of mesh to draw.
    std::vector<VkMesh> m_Meshes;
    /// A set of cloud to draw.
    std::vector<VkCloud> m_Clouds;
    /// Quad mesh used by gradient pass.
    std::unique_ptr<VkMesh> m_Quad;
    /// Optimize cloud to draw.
    std::unique_ptr<VkOptiCloud> m_OptiCloud;

    /// Uniform buffers.
    UniformBuffers m_UniformBuffers;
};
#include "Renderer.h"
#include "Camera.h"
#include "Olympus/Debug.h"
#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan.h>
#include <array>
#include <stdexcept>

//----------------------------------------------------------------------------------------------------------------------
Renderer::Renderer(const olp::Instance &iInstance, VkSurfaceKHR iSurface, uint32_t iWidth, uint32_t iHeight)
    : m_Device(iInstance, iSurface),
      m_Swapchain(m_Device, iWidth, iHeight),
      m_MainPassDescriptor(m_Device),
      m_GradientPassDescriptor(m_Device),
      m_PipelineLayout(m_Device),
      m_GradientPipelineLayout(m_Device),
      m_GradientPipeline(m_Device),
      m_OptiCloudPipeline(m_Device),
      m_ReprojectedPipeline(m_Device),
      m_CloudPipeline(m_Device),
      m_MeshPipeline(m_Device),
      m_PreparePass(m_Device),
      m_DepthBuffer(m_Device),
      m_VertexIndexImage(m_Device)
{
    InitResources();
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::InitResources()
{
    std::cout << "Create ressources" << std::endl;

    InitGeometry();
    CreateSwapchainRessources();
    CreateSyncObjects();
    CreateCommandBuffers();
    UpdatePointSize(2);
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::ReleaseResources()
{
    std::cout << "Release ressources" << std::endl;
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(m_Device.GetDevice(), m_RenderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(m_Device.GetDevice(), m_ImageAvailableSemaphores[i], nullptr);
        vkDestroyFence(m_Device.GetDevice(), m_InFlightFences[i], nullptr);
    }

    for (VkMesh &m : m_Meshes)
        m.Destroy();

    for (VkCloud &c : m_Clouds)
        c.Destroy();

    m_OptiCloud->Destroy();
    m_Quad->Destroy();
    m_Device.Destroy();
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::CreateSwapchainRessources()
{
    std::cout << "Create swapchain ressources" << std::endl;
    CreateDepthBuffer();
    CreateVertexIndexImage();
    CreateRenderPass();

    m_Swapchain.CreateFrameBuffers(m_RenderPass, {m_VertexIndexImage.GetImageView(), m_DepthBuffer.GetImageView()});
    CreatePipelineLayout();
    CreatePipelines();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSets();
    m_OptiCloud->CreateReprojectedBuffer(m_Swapchain.GetImageSize().width, m_Swapchain.GetImageSize().height);
    m_PreparePass.Create(
        m_DescriptorPool,
        *m_OptiCloud,
        m_VertexIndexImage.GetImageView(),
        m_UniformBuffers.ScreenSize,
        m_VertexIndexImage.GetWidth(),
        m_VertexIndexImage.GetHeight());

    ScreenSize sz{m_Swapchain.GetImageSize().width, m_Swapchain.GetImageSize().height};
    m_UniformBuffers.ScreenSize.SendData(&sz, sizeof(sz));
    m_OptiCloud->ResetDraw();
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::RecreateSwapchainResources(uint32_t iWidth, uint32_t iHeight)
{
    std::cout << "Recreate swapchain ressources" << std::endl;
    ReleaseSwapchainResources();

    m_Swapchain.Init(iWidth, iHeight);
    CreateSwapchainRessources();
    CreateCommandBuffers();
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::ReleaseSwapchainResources()
{
    std::cout << "Release swapchain ressources" << std::endl;
    vkDeviceWaitIdle(m_Device.GetDevice());

    for (olp::CommandBuffer &commandBuffer : m_CommandBuffers)
        commandBuffer.Free();

    m_UniformBuffers.Model.Destroy();
    m_UniformBuffers.Camera.Destroy();
    m_UniformBuffers.ScreenSize.Destroy();
    m_UniformBuffers.Lighting.Destroy();
    m_UniformBuffers.PointSize.Destroy();

    vkDestroyDescriptorPool(m_Device.GetDevice(), m_DescriptorPool, nullptr);

    m_MeshPipeline.Destroy();
    m_CloudPipeline.Destroy();
    m_GradientPipeline.Destroy();
    m_OptiCloudPipeline.Destroy();
    m_ReprojectedPipeline.Destroy();
    m_PipelineLayout.Destroy();
    m_GradientPipelineLayout.Destroy();
    vkDestroyRenderPass(m_Device.GetDevice(), m_RenderPass, nullptr);
    m_DepthBuffer.Destroy();
    m_VertexIndexImage.Destroy();
    m_OptiCloud->DestroyReprojectedBuffer();
    m_PreparePass.Destroy();
    m_Swapchain.Destroy();
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::RecreatePipelines()
{
    vkDeviceWaitIdle(m_Device.GetDevice());
    std::cout << "Recreating pipelines" << std::endl;

    CreatePipelines();
    CreateCommandBuffers();
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::InitGeometry()
{

    m_Quad = std::make_unique<VkMesh>(m_Device);
    m_Quad->InitQuad();
    //
    m_OptiCloud = std::make_unique<VkOptiCloud>(m_Device);
    m_OptiCloud->Init();
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::CreateUniformBuffers()
{
    m_UniformBuffers.Model.Init(sizeof(ModelInfo), m_Device);
    m_UniformBuffers.Camera.Init(sizeof(CameraInfo), m_Device);
    m_UniformBuffers.ScreenSize.Init(sizeof(ScreenSize), m_Device);
    m_UniformBuffers.Lighting.Init(sizeof(Lighting), m_Device);
    m_UniformBuffers.PointSize.Init(sizeof(PointSize), m_Device);
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::UpdateUniformBuffers(const glm::mat4 &, const glm::mat4 &)
{
    // TODO
    // CameraInfo cameraUbo{};

    // if (ioCamera.HasUpdated())
    // {
    //     switch (ioCamera.GetCameraType())
    //     {
    //     case CameraType::FreeFly:
    //         cameraUbo.ViewMat = ioCamera.ComputeViewMatrix();
    //         break;

    //     case CameraType::LookAt:
    //         cameraUbo.ViewMat = ioCamera.ComputeLookAt();
    //         break;

    //     case CameraType::Orbit:
    //         cameraUbo.ViewMat = ioCamera.ComputeOrbit();
    //         break;
    //     }

    //     cameraUbo.InvViewMat = ioCamera.ComputeInverseViewMatrix();

    //     ioCamera.SetUpdated(false);
    //     m_OptiCloud->ResetDraw();
    // }
    // else
    // {
    //     cameraUbo.ViewMat = ioCamera.GetViewMatrix();
    //     cameraUbo.InvViewMat = ioCamera.GetInverseViewMatrix();
    // }

    // cameraUbo.ProjMat = ioCamera.GetProjectionMatrix();
    // cameraUbo.ProjMat[1][1] *= -1;

    // cameraUbo.InvProjMat = ioCamera.GetInverseProjectionMatrix();
    // cameraUbo.InvProjMat[1][1] *= -1;

    // cameraUbo.CamPos = ioCamera.GetPosition();

    // ModelInfo modelUbo{};
    // modelUbo.ModelMat = glm::mat4(1.f);
    // modelUbo.MVPMat = cameraUbo.ProjMat * cameraUbo.ViewMat * modelUbo.ModelMat;

    // m_UniformBuffers.Model.SendData(modelUbo, sizeof(modelUbo));
    // m_UniformBuffers.Camera.SendData(cameraUbo, sizeof(cameraUbo));
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::CreatePipelineLayout()
{
    std::vector<VkDescriptorSetLayoutBinding> descriptorBinding(4);

    // Model UBO
    descriptorBinding[0].binding = 0;
    descriptorBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorBinding[0].descriptorCount = 1;
    descriptorBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    descriptorBinding[0].pImmutableSamplers = nullptr;

    // Camera UBO
    descriptorBinding[1].binding = 1;
    descriptorBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorBinding[1].descriptorCount = 1;
    descriptorBinding[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptorBinding[1].pImmutableSamplers = nullptr;

    // Lighting UBO
    descriptorBinding[2].binding = 2;
    descriptorBinding[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorBinding[2].descriptorCount = 1;
    descriptorBinding[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptorBinding[2].pImmutableSamplers = nullptr;

    // Point size UBO
    descriptorBinding[3].binding = 3;
    descriptorBinding[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorBinding[3].descriptorCount = 1;
    descriptorBinding[3].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    descriptorBinding[3].pImmutableSamplers = nullptr;

    m_PipelineLayout.Create(descriptorBinding);

    std::vector<VkDescriptorSetLayoutBinding> descriptorBindingGradient(1);
    // Screen size UBO
    descriptorBindingGradient[0].binding = 0;
    descriptorBindingGradient[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorBindingGradient[0].descriptorCount = 1;
    descriptorBindingGradient[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptorBindingGradient[0].pImmutableSamplers = nullptr;

    m_GradientPipelineLayout.Create(descriptorBindingGradient);
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::CreateDescriptorPool()
{
    VkDescriptorPoolSize uniformPoolSize{};
    uniformPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformPoolSize.descriptorCount = 6; // ModelInfo + CameraInfo + ScreenSize*2 + Lighting + PointSize

    VkDescriptorPoolSize imagePoolSize{};
    imagePoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    imagePoolSize.descriptorCount = 1; // Vertex Index Image

    VkDescriptorPoolSize storageBufferPoolSize{};
    storageBufferPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    storageBufferPoolSize.descriptorCount = 2; // Shuffled buffer + Reproject buffer

    std::array<VkDescriptorPoolSize, 3> poolSizes{uniformPoolSize, imagePoolSize, storageBufferPoolSize};

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 3;

    VK_CHECK_RESULT(vkCreateDescriptorPool(m_Device.GetDevice(), &poolInfo, nullptr, &m_DescriptorPool))
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::CreateDescriptorSets()
{
    m_MainPassDescriptor.AllocateDescriptorSets(m_PipelineLayout.GetDescriptorLayout(), m_DescriptorPool);
    m_MainPassDescriptor.AddWriteDescriptor(0, m_UniformBuffers.Model);
    m_MainPassDescriptor.AddWriteDescriptor(1, m_UniformBuffers.Camera);
    m_MainPassDescriptor.AddWriteDescriptor(2, m_UniformBuffers.Lighting);
    m_MainPassDescriptor.AddWriteDescriptor(3, m_UniformBuffers.PointSize);
    m_MainPassDescriptor.UpdateDescriptorSets();

    m_GradientPassDescriptor.AllocateDescriptorSets(m_GradientPipelineLayout.GetDescriptorLayout(), m_DescriptorPool);
    m_GradientPassDescriptor.AddWriteDescriptor(0, m_UniformBuffers.ScreenSize);
    m_GradientPassDescriptor.UpdateDescriptorSets();
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::CreatePipelines()
{
    std::filesystem::path folder(CLOUD_RENDERING_SHADERS);
    m_GradientPipeline.Create(
        m_GradientPipelineLayout.GetLayout(),
        m_RenderPass,
        0,
        folder / "gradient",
        m_Swapchain.GetImageSize().width,
        m_Swapchain.GetImageSize().height,
        1);

    m_ReprojectedPipeline.Create(
        m_PipelineLayout.GetLayout(),
        m_RenderPass,
        1,
        folder / "reprojectcloud",
        m_Swapchain.GetImageSize().width,
        m_Swapchain.GetImageSize().height,
        2);

    m_OptiCloudPipeline.Create(
        m_PipelineLayout.GetLayout(),
        m_RenderPass,
        1,
        folder / "opticloud",
        m_Swapchain.GetImageSize().width,
        m_Swapchain.GetImageSize().height,
        2);

    m_MeshPipeline.Create(
        m_PipelineLayout.GetLayout(),
        m_RenderPass,
        2,
        folder / "mesh",
        m_Swapchain.GetImageSize().width,
        m_Swapchain.GetImageSize().height,
        1);

    m_CloudPipeline.Create(
        m_PipelineLayout.GetLayout(),
        m_RenderPass,
        2,
        folder / "cloud",
        m_Swapchain.GetImageSize().width,
        m_Swapchain.GetImageSize().height,
        1);
}

//----------------------------------------------------------------------------------------------------------------------
VkFormat Renderer::FindDepthFormat()
{
    return m_Device.FindSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}
//----------------------------------------------------------------------------------------------------------------------
void Renderer::CreateDepthBuffer()
{
    VkFormat depthFormat = FindDepthFormat();

    m_DepthBuffer.Init(m_Swapchain.GetImageSize().width, m_Swapchain.GetImageSize().height, depthFormat);
    m_DepthBuffer.CreateImage(
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        1,
        m_Device.GetMaxUsableSampleCount());
    m_DepthBuffer.CreateImageView(VK_IMAGE_ASPECT_DEPTH_BIT);
    m_DepthBuffer.TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::CreateVertexIndexImage()
{
    m_VertexIndexImage.Init(m_Swapchain.GetImageSize().width, m_Swapchain.GetImageSize().height, VK_FORMAT_R32_SINT);
    m_VertexIndexImage.CreateImage(
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        1,
        VK_SAMPLE_COUNT_1_BIT);
    m_VertexIndexImage.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::CreateRenderPass()
{
    std::array<VkAttachmentDescription, 3> attachments{};

    // Color
    attachments[0].format = m_Swapchain.GetColorFormat();
    attachments[0].samples = m_Device.GetMaxUsableSampleCount();
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // VertexIndex image
    attachments[1].format = m_VertexIndexImage.GetFormat();
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    //Depth
    attachments[2].format = FindDepthFormat();
    attachments[2].samples = m_Device.GetMaxUsableSampleCount();
    attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[2].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorReference{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference depthReference{2, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    std::array<VkSubpassDescription, 3> subpasses{};
    // Gradient subpass
    subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[0].colorAttachmentCount = 1;
    subpasses[0].pColorAttachments = &colorReference;

    // OptiCloud pass
    std::array<VkAttachmentReference, 2> colorReferencesSub2{};
    colorReferencesSub2[0] = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    colorReferencesSub2[1] = {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[1].colorAttachmentCount = static_cast<uint32_t>(colorReferencesSub2.size());
    subpasses[1].pColorAttachments = colorReferencesSub2.data();
    subpasses[1].pDepthStencilAttachment = &depthReference;

    // Final pass (Mesh + Cloud)
    subpasses[2].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[2].colorAttachmentCount = 1;
    subpasses[2].pColorAttachments = &colorReference;
    subpasses[2].pDepthStencilAttachment = &depthReference;

    std::array<VkSubpassDependency, 3> dependencies{};
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = 0;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT; // Framebuffer local

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = 1;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[2].srcSubpass = 1;
    dependencies[2].dstSubpass = 2;
    dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[2].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[2].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
    renderPassInfo.pSubpasses = subpasses.data();
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    VK_CHECK_RESULT(vkCreateRenderPass(m_Device.GetDevice(), &renderPassInfo, nullptr, &m_RenderPass))
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::CreateCommandBuffers()
{
    m_CommandBuffers.clear();
    m_CommandBuffers.reserve(m_Swapchain.GetImageCount());

    for (size_t i = 0; i < m_CommandBuffers.capacity(); ++i)
    {
        m_CommandBuffers.emplace_back(m_Device);
    }
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::BuildCommandBuffer(uint32_t iIndex)
{
    olp::CommandBuffer &commandBuffer = m_CommandBuffers[iIndex];
    commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    const VkExtent2D imageSize = m_Swapchain.GetImageSize();
    std::array<VkClearValue, 3> clearValues{};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].color = {-1};
    clearValues[2].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_RenderPass;
    renderPassInfo.framebuffer = m_Swapchain.GetFramebuffer(iIndex);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = {imageSize};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer.GetBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(
        commandBuffer.GetBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_GradientPipeline.GetPipeline());

    vkCmdBindDescriptorSets(
        commandBuffer.GetBuffer(),
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_GradientPipelineLayout.GetLayout(),
        0,
        1,
        &m_GradientPassDescriptor.GetDescriptorSet(),
        0,
        nullptr);

    m_Quad->Draw(commandBuffer.GetBuffer());
    vkCmdNextSubpass(commandBuffer.GetBuffer(), VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(
        commandBuffer.GetBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_ReprojectedPipeline.GetPipeline());

    vkCmdBindDescriptorSets(
        commandBuffer.GetBuffer(),
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_PipelineLayout.GetLayout(),
        0,
        1,
        &m_MainPassDescriptor.GetDescriptorSet(),
        0,
        nullptr);

    m_OptiCloud->DrawReprojectedBuffer(commandBuffer.GetBuffer());

    vkCmdBindPipeline(
        commandBuffer.GetBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_OptiCloudPipeline.GetPipeline());

    vkCmdBindDescriptorSets(
        commandBuffer.GetBuffer(),
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_PipelineLayout.GetLayout(),
        0,
        1,
        &m_MainPassDescriptor.GetDescriptorSet(),
        0,
        nullptr);

    m_OptiCloud->DrawVertexBuffer(commandBuffer.GetBuffer());

    vkCmdNextSubpass(commandBuffer.GetBuffer(), VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer.GetBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_MeshPipeline.GetPipeline());

    vkCmdBindDescriptorSets(
        commandBuffer.GetBuffer(),
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_PipelineLayout.GetLayout(),
        0,
        1,
        &m_MainPassDescriptor.GetDescriptorSet(),
        0,
        nullptr);

    for (VkMesh &mesh : m_Meshes)
        mesh.Draw(commandBuffer.GetBuffer());

    vkCmdBindPipeline(
        commandBuffer.GetBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_CloudPipeline.GetPipeline());

    vkCmdBindDescriptorSets(
        commandBuffer.GetBuffer(),
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_PipelineLayout.GetLayout(),
        0,
        1,
        &m_MainPassDescriptor.GetDescriptorSet(),
        0,
        nullptr);

    for (VkCloud &cloud : m_Clouds)
        cloud.Draw(commandBuffer.GetBuffer());

    vkCmdEndRenderPass(commandBuffer.GetBuffer());

    commandBuffer.End();
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::CreateSyncObjects()
{
    m_ImagesInFlight.resize(m_Swapchain.GetImageCount(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        VK_CHECK_RESULT(
            vkCreateSemaphore(m_Device.GetDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]))
        VK_CHECK_RESULT(
            vkCreateSemaphore(m_Device.GetDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]))
        VK_CHECK_RESULT(vkCreateFence(m_Device.GetDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]))
    }
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::Enable3PointLighting(bool iEnabled)
{
    m_UniformBuffers.Lighting.SendData(&iEnabled, sizeof(bool), offsetof(Lighting, ThreePointEnabled));
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::SetLightColor(const glm::vec3 &iLightColor)
{
    m_UniformBuffers.Lighting.SendData(&iLightColor, sizeof(glm::vec3), offsetof(Lighting, Color));
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::SetLightIntensity(float iIntensity)
{
    m_UniformBuffers.Lighting.SendData(&iIntensity, sizeof(float), offsetof(Lighting, Intensity));
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::UpdatePointSize(uint32_t iPointSize)
{
    m_UniformBuffers.PointSize.SendData(&iPointSize, sizeof(iPointSize));
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::UpdatePointsByStep(uint32_t iPointCount)
{
    m_OptiCloud->SetPointsByStep(iPointCount);
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::DrawNextFrame(const glm::mat4 &iView, const glm::mat4 &iProj)
{
    m_PreparePass.WaitFence();
    vkWaitForFences(m_Device.GetDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = m_Swapchain.GetNextImage(m_ImageAvailableSemaphores[m_CurrentFrame], imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapchainResources(m_Swapchain.GetImageSize().width, m_Swapchain.GetImageSize().height);
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    // Check if a previous frame is using this image (i.e. there is its fence to wait on)
    if (m_ImagesInFlight[imageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(m_Device.GetDevice(), 1, &m_ImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    // Mark the image as now being in use by this frame
    m_ImagesInFlight[imageIndex] = m_InFlightFences[m_CurrentFrame];

    BuildCommandBuffer(imageIndex);
    UpdateUniformBuffers(iView, iProj);

    std::array<VkPipelineStageFlags, 1> waitStages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    std::array<VkSemaphore, 1> waitSemaphores = {m_ImageAvailableSemaphores[m_CurrentFrame]};
    std::array<VkSemaphore, 1> signalSemaphores = {m_PreparePass.GetSemaphore()};

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_CommandBuffers[imageIndex].GetBuffer();
    submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
    submitInfo.pSignalSemaphores = signalSemaphores.data();

    vkResetFences(m_Device.GetDevice(), 1, &m_InFlightFences[m_CurrentFrame]);
    VK_CHECK_RESULT(
        vkQueueSubmit(m_Device.GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]))

    m_PreparePass.Process(m_PreparePass.GetSemaphore(), m_RenderFinishedSemaphores[m_CurrentFrame]);

    result = m_Swapchain.PresentNextImage(&m_RenderFinishedSemaphores[m_CurrentFrame], imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        RecreateSwapchainResources(m_Swapchain.GetImageSize().width, m_Swapchain.GetImageSize().height);
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    //	m_VulkanWindow->frameReady();
    //	m_VulkanWindow->requestUpdate();
}
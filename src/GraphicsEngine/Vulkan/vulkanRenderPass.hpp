#pragma once

#include "vulkanGraphicsEngineLink.hpp"
#include "vulkanImage.hpp"

#include <deque>
#include <functional>
/**@todo: Rework this class to be like the others and depend on a VulkanFramebuffer class.
 * - MEDIUM PRIORITY: Important to keep code consistent, and to ease development in the future, but not crucial to make the engine work.
 */
class VulkanRenderPass {
public:
    VkRenderPass renderPass{};
    std::vector<VkFramebuffer> framebuffers{};
    std::vector<VkClearValue> clearValues{};

    void destroy() {
        for (std::function<void()>& function : framebufferDeletionQueue) { function(); }
        framebufferDeletionQueue.clear();
        for (std::function<void()>& function : renderPassDeletionQueue) { function(); }
        renderPassDeletionQueue.clear();
    }

    void create(VulkanGraphicsEngineLink *engineLink) {
        //destroy all old stuffs
        for (std::function<void()>& function : renderPassDeletionQueue) { function(); }
        renderPassDeletionQueue.clear();
        //update engine link
        linkedRenderEngine = engineLink;
        //create renderPass
        VkAttachmentDescription colorAttachmentDescription{};
        colorAttachmentDescription.format = linkedRenderEngine->swapchain->image_format;
        colorAttachmentDescription.samples = linkedRenderEngine->settings->msaaSamples;
        colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentDescription.finalLayout = linkedRenderEngine->settings->msaaSamples == VK_SAMPLE_COUNT_1_BIT ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkAttachmentDescription depthAttachmentDescription{};
        depthAttachmentDescription.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
        depthAttachmentDescription.samples = linkedRenderEngine->settings->msaaSamples;
        depthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        VkAttachmentDescription colorResolveAttachmentDescription{};
        colorResolveAttachmentDescription.format = linkedRenderEngine->swapchain->image_format;
        colorResolveAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        colorResolveAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorResolveAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorResolveAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorResolveAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorResolveAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorResolveAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        VkAttachmentReference colorAttachmentReference{};
        colorAttachmentReference.attachment = 0;
        colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkAttachmentReference depthAttachmentReference{};
        depthAttachmentReference.attachment = 1;
        depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        VkAttachmentReference colorResolveAttachmentReference{};
        colorResolveAttachmentReference.attachment = 2;
        colorResolveAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkSubpassDescription subpassDescription{};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorAttachmentReference;
        subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
        subpassDescription.pResolveAttachments = linkedRenderEngine->settings->msaaSamples == VK_SAMPLE_COUNT_1_BIT ? nullptr : &colorResolveAttachmentReference;
        VkSubpassDependency subpassDependency{};
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0;
        subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpassDependency.srcAccessMask = 0;
        subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        std::vector<VkAttachmentDescription> attachmentDescriptions{};
        if (linkedRenderEngine->settings->msaaSamples != VK_SAMPLE_COUNT_1_BIT) { attachmentDescriptions = {colorAttachmentDescription, depthAttachmentDescription, colorResolveAttachmentDescription}; } else { attachmentDescriptions = {colorAttachmentDescription, depthAttachmentDescription}; }
        VkRenderPassCreateInfo renderPassCreateInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
        renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpassDescription;
        renderPassCreateInfo.dependencyCount = 1;
        renderPassCreateInfo.pDependencies = &subpassDependency;
        if (vkCreateRenderPass(linkedRenderEngine->device->device, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) { throw std::runtime_error("failed to create render pass!"); }
        renderPassDeletionQueue.emplace_front([&]{ vkDestroyRenderPass(linkedRenderEngine->device->device, renderPass, nullptr); });
    }

    void createFramebuffers() {
        for (std::function<void()>& function : framebufferDeletionQueue) { function(); }
        framebufferDeletionQueue.clear();
        //Set clear values based on multisampling settings
        clearValues.resize(linkedRenderEngine->settings->msaaSamples == VK_SAMPLE_COUNT_1_BIT ? 2 : 3);
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        if (linkedRenderEngine->settings->msaaSamples != VK_SAMPLE_COUNT_1_BIT) { clearValues[2].color = {0.0f, 0.0f, 0.0f, 1.0f}; }
        //Create framebuffer images
        VulkanImage::CreateInfo colorImageCreateInfo{linkedRenderEngine->swapchain->image_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY};
        colorImageCreateInfo.msaaSamples = linkedRenderEngine->settings->msaaSamples;
        colorImage.create(linkedRenderEngine, &colorImageCreateInfo);
        framebufferDeletionQueue.emplace_front([&]{ colorImage.destroy(); });
        VulkanImage::CreateInfo depthImageCreateInfo{VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY};
        depthImageCreateInfo.msaaSamples = linkedRenderEngine->settings->msaaSamples;
        depthImageCreateInfo.imageType = VULKAN_DEPTH;
        depthImageCreateInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthImage.create(linkedRenderEngine, &depthImageCreateInfo);
        framebufferDeletionQueue.emplace_front([&]{ depthImage.destroy(); });
        //create framebuffers
        framebuffers.resize(linkedRenderEngine->swapchain->image_count);
        std::vector<VkImageView> framebufferAttachments{};
        for (unsigned int i = 0; i < framebuffers.size(); i++) {
            if (linkedRenderEngine->settings->msaaSamples == VK_SAMPLE_COUNT_1_BIT) { framebufferAttachments = {(*linkedRenderEngine->swapchainImageViews)[i], depthImage.view}; }
            else { framebufferAttachments = {colorImage.view, depthImage.view, (*linkedRenderEngine->swapchainImageViews)[i]}; }
            VkFramebufferCreateInfo framebufferCreateInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
            framebufferCreateInfo.renderPass = renderPass;
            framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(framebufferAttachments.size());
            framebufferCreateInfo.pAttachments = framebufferAttachments.data();
            framebufferCreateInfo.width = linkedRenderEngine->swapchain->extent.width;
            framebufferCreateInfo.height = linkedRenderEngine->swapchain->extent.height;
            framebufferCreateInfo.layers = 1;
            if (vkCreateFramebuffer(linkedRenderEngine->device->device, &framebufferCreateInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) { throw std::runtime_error("failed to create framebuffers!"); }
        }
        framebufferDeletionQueue.emplace_front([&]{ for (VkFramebuffer framebuffer : framebuffers) { vkDestroyFramebuffer(linkedRenderEngine->device->device, framebuffer, nullptr); } });
    }

    VkRenderPassBeginInfo beginRenderPass(unsigned int framebufferIndex = 0) {
        VkRenderPassBeginInfo renderPassBeginInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        renderPassBeginInfo.renderArea.offset = {0, 0};
        renderPassBeginInfo.renderArea.extent = linkedRenderEngine->swapchain->extent;
        renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassBeginInfo.pClearValues = clearValues.data();
        renderPassBeginInfo.renderPass = renderPass;
        if (framebufferIndex > framebuffers.size()) { throw std::runtime_error(std::string("framebuffers[") + std::to_string(framebufferIndex) + "] does not exist!"); }
        renderPassBeginInfo.framebuffer = framebuffers[framebufferIndex];
        return renderPassBeginInfo;
    }

private:
    std::deque<std::function<void()>> framebufferDeletionQueue{};
    std::deque<std::function<void()>> renderPassDeletionQueue{};
    VulkanGraphicsEngineLink *linkedRenderEngine{};
    VulkanImage colorImage{};
    VulkanImage depthImage{};
};

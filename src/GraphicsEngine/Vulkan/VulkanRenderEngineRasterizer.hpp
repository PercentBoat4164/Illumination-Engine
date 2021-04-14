#pragma once

#include <functional>
#include <deque>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <VkBootstrap.h>

#include "VulkanRenderEngine.hpp"

class VulkanRenderEngineRasterizer : public VulkanRenderEngine {
public:
    bool update() override {
        //GPU synchronization
        if (window == nullptr) { return false; }
        if (assets.empty()) { return glfwWindowShouldClose(window) != 1; }
        vkWaitForFences(device.device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        uint32_t imageIndex = 0;
        VkResult result = vkAcquireNextImageKHR(device.device, swapchain.swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            createSwapchain();
            return glfwWindowShouldClose(window) != 1;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { throw std::runtime_error("failed to acquire swapchain image!"); }
        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) { vkWaitForFences(device.device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX); }
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];
        //update state of frame
        VkDeviceSize offsets[] = {0};
        //record command buffers for color pass
        commandBufferManager.resetCommandBuffer((int)(imageIndex + (swapchain.image_count - 1)) % (int)swapchain.image_count);
        commandBufferManager.recordCommandBuffer((int)imageIndex);
        VkViewport viewport{};
        viewport.x = 0.f;
        viewport.y = 0.f;
        viewport.width = (float)swapchain.extent.width;
        viewport.height = (float)swapchain.extent.height;
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;
        vkCmdSetViewport(commandBufferManager.commandBuffers[imageIndex], 0, 1, &viewport);
        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapchain.extent;
        vkCmdSetScissor(commandBufferManager.commandBuffers[imageIndex], 0, 1, &scissor);
        std::vector<VkClearValue> clearValues{static_cast<size_t>(settings.msaaSamples == VK_SAMPLE_COUNT_1_BIT ? 2 : 3)};
        clearValues[0].depthStencil = {1.0f, 0};
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        if (settings.msaaSamples != VK_SAMPLE_COUNT_1_BIT) { clearValues[2].color = {0.0f, 0.0f, 0.0f, 1.0f}; }
        renderPassManager.clearValues = clearValues;
        VkRenderPassBeginInfo renderPassBeginInfo = renderPassManager.beginRenderPass(imageIndex);
        vkCmdBeginRenderPass(commandBufferManager.commandBuffers[imageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        camera.update();
        for (Asset *asset : assets) {
            if (asset->render) {
                //update asset
                asset->update(camera);
                //record command buffer for this asset
                vkCmdBindVertexBuffers(commandBufferManager.commandBuffers[imageIndex], 0, 1, &asset->vertexBuffer.buffer, offsets);
                vkCmdBindIndexBuffer(commandBufferManager.commandBuffers[imageIndex], asset->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
                vkCmdBindDescriptorSets(commandBufferManager.commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, asset->pipelineManagers[0].pipelineLayout, 0, 1, &asset->pipelineManagers[0].descriptorSet, 0, nullptr);
                vkCmdBindPipeline(commandBufferManager.commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, asset->pipelineManagers[0].pipeline);
                vkCmdDrawIndexed(commandBufferManager.commandBuffers[imageIndex], asset->indices.size(), 1, 0, 0, 0);
            }
        }
        vkCmdEndRenderPass(commandBufferManager.commandBuffers[imageIndex]);
        if (vkEndCommandBuffer(commandBufferManager.commandBuffers[imageIndex]) != VK_SUCCESS) { throw std::runtime_error("failed to record command buffer!"); }
        //Submit
        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBufferManager.commandBuffers[imageIndex];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        vkResetFences(device.device, 1, &inFlightFences[currentFrame]);
        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) { throw std::runtime_error("failed to submit draw command buffer!"); }
        //Present
        VkSwapchainKHR swapchains[] = {swapchain.swapchain};
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &imageIndex;
        result = vkQueuePresentKHR(presentQueue, &presentInfo);
        //update frameTime and frameNumber
        auto currentTime = (float)glfwGetTime();
        frameTime = currentTime - previousTime;
        previousTime = currentTime;
        frameNumber++;
        //Check if window has been resized
        vkQueueWaitIdle(presentQueue);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            createSwapchain();
        } else if (result != VK_SUCCESS) { throw std::runtime_error("failed to present swapchain image!"); }
        currentFrame = (currentFrame + 1) % settings.MAX_FRAMES_IN_FLIGHT;
        return glfwWindowShouldClose(window) != 1;
    }

    size_t currentFrame{};
    bool framebufferResized{false};
    float previousTime{};
    float frameTime{};
    int frameNumber{};
};
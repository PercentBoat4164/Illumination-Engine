/* Include this file's header. */
#include "IEDescriptorSet.hpp"

/* Include dependencies within this module. */
#include "Buffer/Buffer.hpp"
#include "IERenderEngine.hpp"
#include "Image/Image.hpp"
#include "Image/TextureVulkan.hpp"

/* Include system dependencies. */
#include <cassert>

void IEDescriptorSet::destroy() {
    for (const std::function<void()> &function : deletionQueue) function();
    deletionQueue.clear();
}

void IEDescriptorSet::create(IERenderEngine *renderEngineLink, IEDescriptorSet::CreateInfo *createInfo) {
    linkedRenderEngine = renderEngineLink;
    createdWith        = *createInfo;
    assert(!createdWith.data.empty());
    assert(
      createdWith.data.size() == createdWith.shaderStages.size() &&
      createdWith.data.size() == createdWith.poolSizes.size()
    );
    std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings{};
    descriptorSetLayoutBindings.reserve(createdWith.poolSizes.size());
    for (unsigned long i = 0; i < createdWith.poolSizes.size(); ++i) {
        VkDescriptorSetLayoutBinding descriptorSetLayoutBinding{};
        descriptorSetLayoutBinding.descriptorType  = createdWith.poolSizes[i].type;
        descriptorSetLayoutBinding.binding         = i;
        descriptorSetLayoutBinding.stageFlags      = createdWith.shaderStages[i];
        descriptorSetLayoutBinding.descriptorCount = createdWith.poolSizes[i].descriptorCount;
        descriptorSetLayoutBindings.push_back(descriptorSetLayoutBinding);
    }
    std::vector<VkDescriptorBindingFlagsEXT> flags{};
    flags.resize(createdWith.data.size());
    flags[flags.size() - 1] = createdWith.flags;
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT descriptorSetLayoutBindingFlagsCreateInfo{
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT};
    descriptorSetLayoutBindingFlagsCreateInfo.bindingCount =
      static_cast<uint32_t>(descriptorSetLayoutBindings.size());
    descriptorSetLayoutBindingFlagsCreateInfo.pBindingFlags = flags.data();
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    if (linkedRenderEngine->extensionAndFeatureInfo.queryFeatureSupport(
          IE_ENGINE_FEATURE_QUERY_VARIABLE_DESCRIPTOR_COUNT
        )) {
        descriptorSetLayoutCreateInfo.pNext = &descriptorSetLayoutBindingFlagsCreateInfo;
    }
    descriptorSetLayoutCreateInfo.pBindings    = descriptorSetLayoutBindings.data();
    descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());
    if (vkCreateDescriptorSetLayout(linkedRenderEngine->device.m_device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor m_layout!");
    }
    deletionQueue.emplace_back([&] {
        vkDestroyDescriptorSetLayout(linkedRenderEngine->device.m_device, descriptorSetLayout, nullptr);
    });
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(createdWith.poolSizes.size());
    descriptorPoolCreateInfo.pPoolSizes    = createdWith.poolSizes.data();
    descriptorPoolCreateInfo.maxSets       = 1;
    if (vkCreateDescriptorPool(linkedRenderEngine->device.m_device, &descriptorPoolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
    deletionQueue.emplace_back([&] {
        vkDestroyDescriptorPool(linkedRenderEngine->device.m_device, descriptorPool, nullptr);
    });
    VkDescriptorSetVariableDescriptorCountAllocateInfoEXT descriptorSetVariableDescriptorCountAllocateInfo{
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT};
    descriptorSetVariableDescriptorCountAllocateInfo.descriptorSetCount = 1;
    descriptorSetVariableDescriptorCountAllocateInfo.pDescriptorCounts  = &createdWith.maxIndex;
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    if (linkedRenderEngine->extensionAndFeatureInfo.queryFeatureSupport(
          IE_ENGINE_FEATURE_QUERY_VARIABLE_DESCRIPTOR_COUNT
        )) {
        descriptorSetAllocateInfo.pNext = &descriptorSetVariableDescriptorCountAllocateInfo;
    }
    descriptorSetAllocateInfo.descriptorPool     = descriptorPool;
    descriptorSetAllocateInfo.pSetLayouts        = &descriptorSetLayout;
    descriptorSetAllocateInfo.descriptorSetCount = 1;
    if (vkAllocateDescriptorSets(linkedRenderEngine->device.m_device, &descriptorSetAllocateInfo, &descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor set!");
    }
    std::vector<int> bindings{};
    bindings.reserve(createdWith.data.size());
    std::vector<std::optional<std::variant<IE::Graphics::Image *, Buffer *>>> data{};
    data.reserve(createdWith.data.size());
    for (size_t i = 0; i < createdWith.data.size(); ++i) {
        if (createdWith.data[i].has_value()) {
            bindings.push_back(i);
            data.push_back(createdWith.data[i]);
        }
    }
    update(data, bindings);
}

void IEDescriptorSet::update(
  std::vector<std::optional<std::variant<IE::Graphics::Image *, Buffer *>>> newData,
  std::vector<int>                                                            bindings
) {
    std::vector<VkDescriptorImageInfo> imageDescriptorInfos{};
    imageDescriptorInfos.reserve(createdWith.poolSizes.size());
    std::vector<VkDescriptorBufferInfo> bufferDescriptorInfos{};
    bufferDescriptorInfos.reserve(createdWith.poolSizes.size());
    if (bindings.empty()) bindings.resize(newData.size(), 0);
    std::vector<VkWriteDescriptorSet> descriptorWrites{};
    descriptorWrites.resize(bindings.size());
    for (unsigned long i = 0; i < bindings.size(); ++i) {
        if (newData[i].has_value()) {
            VkWriteDescriptorSet writeDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            writeDescriptorSet.dstSet         = descriptorSet;
            writeDescriptorSet.descriptorType = createdWith.poolSizes[bindings[i]].type;
            writeDescriptorSet.dstBinding     = bindings[i];
            writeDescriptorSet.descriptorCount =
              writeDescriptorSet.dstBinding == createdWith.data.size() ? createdWith.maxIndex : 1;
            if (writeDescriptorSet.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
                VkDescriptorImageInfo storageImageDescriptorInfo{};
                storageImageDescriptorInfo.imageView = dynamic_cast<IE::Graphics::detail::TextureVulkan *>(
                                                         std::get<IE::Graphics::Image *>(newData[i].value())
                )
                                                         ->m_view;
                storageImageDescriptorInfo.sampler = dynamic_cast<IE::Graphics::detail::TextureVulkan *>(
                                                       std::get<IE::Graphics::Image *>(newData[i].value())
                )
                                                       ->m_sampler;
                storageImageDescriptorInfo.imageLayout = dynamic_cast<IE::Graphics::detail::TextureVulkan *>(
                                                           std::get<IE::Graphics::Image *>(newData[i].value())
                )
                                                           ->m_layout;
                if (storageImageDescriptorInfo.imageView == VK_NULL_HANDLE)
                    throw std::runtime_error("no image given or given image does not have an associated view!");
                imageDescriptorInfos.push_back(storageImageDescriptorInfo);
                writeDescriptorSet.pImageInfo = &imageDescriptorInfos[imageDescriptorInfos.size() - 1];
            } else if (writeDescriptorSet.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                VkDescriptorImageInfo combinedImageSamplerDescriptorInfo{};
                combinedImageSamplerDescriptorInfo.imageView =
                  dynamic_cast<IE::Graphics::detail::TextureVulkan *>(
                    std::get<IE::Graphics::Image *>(newData[i].value())
                  )
                    ->m_view;
                combinedImageSamplerDescriptorInfo.sampler = dynamic_cast<IE::Graphics::detail::TextureVulkan *>(
                                                               std::get<IE::Graphics::Image *>(newData[i].value())
                )
                                                               ->m_sampler;
                combinedImageSamplerDescriptorInfo.imageLayout =
                  dynamic_cast<IE::Graphics::detail::TextureVulkan *>(
                    std::get<IE::Graphics::Image *>(newData[i].value())
                  )
                    ->m_layout;
                if (combinedImageSamplerDescriptorInfo.sampler == VK_NULL_HANDLE)
                    throw std::runtime_error("no image given or given image does not have an associated sampler!");
                imageDescriptorInfos.push_back(combinedImageSamplerDescriptorInfo);
                writeDescriptorSet.pImageInfo = &imageDescriptorInfos[imageDescriptorInfos.size() - 1];
            } else if (writeDescriptorSet.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
                VkDescriptorBufferInfo storageBufferDescriptorInfo{};
                storageBufferDescriptorInfo.buffer = std::get<Buffer *>(newData[i].value())->buffer;
                storageBufferDescriptorInfo.offset = 0;
                storageBufferDescriptorInfo.range  = VK_WHOLE_SIZE;
                if (storageBufferDescriptorInfo.buffer == VK_NULL_HANDLE)
                    throw std::runtime_error("no Buffer given or given Buffer has not been created!");
                bufferDescriptorInfos.push_back(storageBufferDescriptorInfo);
                writeDescriptorSet.pBufferInfo = &bufferDescriptorInfos[bufferDescriptorInfos.size() - 1];
            } else if (writeDescriptorSet.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                VkDescriptorBufferInfo uniformBufferDescriptorInfo{};
                uniformBufferDescriptorInfo.buffer = std::get<Buffer *>(newData[i].value())->buffer;
                uniformBufferDescriptorInfo.offset = 0;
                uniformBufferDescriptorInfo.range  = VK_WHOLE_SIZE;
                if (uniformBufferDescriptorInfo.buffer == VK_NULL_HANDLE)
                    throw std::runtime_error("no Buffer given or given Buffer has not been created!");
                bufferDescriptorInfos.push_back(uniformBufferDescriptorInfo);
                writeDescriptorSet.pBufferInfo = &bufferDescriptorInfos[bufferDescriptorInfos.size() - 1];
            } else {
                throw std::runtime_error(
                  "unsupported descriptor type: " + std::to_string(writeDescriptorSet.descriptorType)
                );
            }
            descriptorWrites[i] = writeDescriptorSet;
        }
    }
    if (!descriptorWrites.empty()) {
        vkUpdateDescriptorSets(
          linkedRenderEngine->device.m_device,
          descriptorWrites.size(),
          descriptorWrites.data(),
          0,
          nullptr
        );
    }
}

IEDescriptorSet::~IEDescriptorSet() {
    destroy();
}

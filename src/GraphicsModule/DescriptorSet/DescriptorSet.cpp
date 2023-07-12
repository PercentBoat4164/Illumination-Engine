#include "DescriptorSet.hpp"

#include "GraphicsModule/RenderPass/Subpass.hpp"
#include "RenderEngine.hpp"

#include <array>
#include <iostream>
#include <vector>

void IE::Graphics::DescriptorSet::build(
  IE::Graphics::Subpass                              *t_subpass,
  std::vector<std::shared_ptr<IE::Graphics::Shader>> &t_shaders
) {
    m_linkedRenderEngine = t_subpass->m_linkedRenderEngine;

    std::vector<VkDescriptorPoolSize> poolSizes;

    for (std::shared_ptr<Shader> &shader : t_shaders) {
        std::vector<Shader::ReflectionInfo> reflectionInfo = shader->getReflectionInfo();
        for (Shader::ReflectionInfo &info : reflectionInfo) {
            // Does this resource belong to this set?
            if (info.set == m_type) {
                bool typeAlreadyExists{};
                // Has a resource of this type already been added to this set?
                for (VkDescriptorPoolSize &poolSize : poolSizes) {
                    if (poolSize.type == info.type) {
                        ++poolSize.descriptorCount;
                        typeAlreadyExists = true;
                        break;
                    }
                }
                if (!typeAlreadyExists) poolSizes.push_back({.type = info.type, .descriptorCount = 1});
            }
        }
    }

    VkDescriptorPoolCreateInfo poolCreateInfo{
      .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .pNext         = nullptr,
      .flags         = 0x0,
      .maxSets       = 0x1,
      .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
      .pPoolSizes    = poolSizes.data()};

    VkResult result{
      vkCreateDescriptorPool(m_linkedRenderEngine->m_device.device, &poolCreateInfo, nullptr, &m_pool)};
    if (result != VK_SUCCESS)
        m_linkedRenderEngine->getLogger().log(
          "Failed to create descriptor pool with error: " +
          IE::Graphics::RenderEngine::translateVkResultCodes(result)
        );
    else m_linkedRenderEngine->getLogger().log("Created Descriptor Pool");
}

void IE::Graphics::DescriptorSet::build(IE::Graphics::RenderEngine *t_engineLink) {
    m_linkedRenderEngine = t_engineLink;

    std::vector<VkDescriptorPoolSize> poolSizes{};
    for (auto descriptor : PER_FRAME_DESCRIPTOR_SET_LAYOUT_INFO) {
        bool typeAlreadyExists{};
        for (VkDescriptorPoolSize &poolSize : poolSizes) {
            if (get<0>(descriptor) == poolSize.type) {
                ++poolSize.descriptorCount;
                typeAlreadyExists = true;
                break;
            }
        }
        if (!typeAlreadyExists) poolSizes.push_back({.type = get<0>(descriptor), .descriptorCount = 1});
    }

    VkDescriptorPoolCreateInfo poolCreateInfo{
      .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .pNext         = nullptr,
      .flags         = 0x0,
      .maxSets       = 0x1,
      .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
      .pPoolSizes    = poolSizes.data()};

    VkResult result{vkCreateDescriptorPool(t_engineLink->m_device.device, &poolCreateInfo, nullptr, &m_pool)};
    if (result != VK_SUCCESS)
        t_engineLink->getLogger().log(
          "Failed to create descriptor pool with error: " +
          IE::Graphics::RenderEngine::translateVkResultCodes(result)
        );
    else t_engineLink->getLogger().log("Created Descriptor Pool");
}

VkDescriptorSetLayout IE::Graphics::DescriptorSet::getLayout(
  IE::Graphics::RenderEngine          *t_engineLink,
  size_t                               t_set,
  std::vector<std::shared_ptr<Shader>> t_shaders
) {
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
    VkDescriptorSetLayout                     layout;
    if (t_set != 0) {
        for (std::shared_ptr<Shader> &shader : t_shaders)
            for (Shader::ReflectionInfo &info : shader->reflectedInfo)
                if (info.set == t_set)
                    layoutBindings.push_back(
                      {.binding         = info.binding,
                       .descriptorType  = info.type,
                       .descriptorCount = 1,  // Indicates m_size of array in shader. Arrays are not supported yet.
                       .stageFlags      = static_cast<VkShaderStageFlags>(shader->m_stage),
                       .pImmutableSamplers = nullptr}
                    );
    } else {
        for (auto descriptor : PER_FRAME_DESCRIPTOR_SET_LAYOUT_INFO)
            layoutBindings.push_back(
              {.binding            = get<1>(descriptor),
               .descriptorType     = get<0>(descriptor),
               .descriptorCount    = 1,  // Indicates m_size of array in shader. Arrays are not supported yet.
               .stageFlags         = VK_SHADER_STAGE_ALL_GRAPHICS,
               .pImmutableSamplers = nullptr}
            );
    }
    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{
      .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext        = nullptr,
      .flags        = 0x0,
      .bindingCount = static_cast<uint32_t>(layoutBindings.size()),
      .pBindings    = layoutBindings.data()};
    VkResult result{
      vkCreateDescriptorSetLayout(t_engineLink->m_device.device, &layoutCreateInfo, nullptr, &layout)};
    if (result != VK_SUCCESS)
        t_engineLink->getLogger().log(
          "Failed to create descriptor set layout! Error: " +
          IE::Graphics::RenderEngine::translateVkResultCodes(result)
        );
    else t_engineLink->getLogger().log("Created Descriptor Set Layout");
    return layout;
}

IE::Graphics::DescriptorSet::DescriptorSet(IE::Graphics::DescriptorSet::SetType t_type) : m_type(t_type) {
}

const std::vector<std::tuple<VkDescriptorType, uint32_t>>
  IE::Graphics::DescriptorSet::PER_FRAME_DESCRIPTOR_SET_LAYOUT_INFO{
    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0},
    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}
};

void IE::Graphics::DescriptorSet::destroy() {
    if (m_pool) {
        vkDestroyDescriptorPool(m_linkedRenderEngine->m_device.device, m_pool, nullptr);
        m_pool = VK_NULL_HANDLE;
    }
}

IE::Graphics::DescriptorSet::~DescriptorSet() {
    destroy();
}

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
    m_subpass = t_subpass;

    std::vector<VkDescriptorPoolSize> poolSizes;

    for (std::shared_ptr<Shader> &shader : t_shaders) {
        std::vector<Shader::ReflectionInfo> reflectionInfo = shader->getReflectionInfo();
        for (Shader::ReflectionInfo &info : reflectionInfo) {
            // Does this resource belong to this set?
            if (info.set == m_setNumber) {
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

    VkResult result{vkCreateDescriptorPool(
      m_subpass->m_renderPass->m_renderPassSeries->m_linkedRenderEngine->m_device.device,
      &poolCreateInfo,
      nullptr,
      &m_pool
    )};
    if (result != VK_SUCCESS)
        m_subpass->m_renderPass->m_renderPassSeries->m_linkedRenderEngine->getLogger().log(
          "Failed to create descriptor pool with error: " +
          IE::Graphics::RenderEngine::translateVkResultCodes(result)
        );
    else
        m_subpass->m_renderPass->m_renderPassSeries->m_linkedRenderEngine->getLogger().log(
          "Created Descriptor Pool"
        );
}

void IE::Graphics::DescriptorSet::build(IE::Graphics::RenderEngine *t_engineLink) {
    VkDescriptorPoolCreateInfo poolCreateInfo{
      .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .pNext         = nullptr,
      .flags         = 0x0,
      .maxSets       = 0x1,
      .poolSizeCount = static_cast<uint32_t>(DESCRIPTOR_POOL_SIZES[m_setNumber].size()),
      .pPoolSizes    = DESCRIPTOR_POOL_SIZES[m_setNumber].data()};

    VkResult result{vkCreateDescriptorPool(t_engineLink->m_device.device, &poolCreateInfo, nullptr, &m_pool)};
    if (result != VK_SUCCESS)
        t_engineLink->getLogger().log(
          "Failed to create descriptor pool with error: " +
          IE::Graphics::RenderEngine::translateVkResultCodes(result)
        );
    else t_engineLink->getLogger().log("Created Descriptor Pool");
}

bool IE::Graphics::DescriptorSet::isDescriptorControlledBySetType(
  IE::Graphics::DescriptorSet::SetNumber t_type,
  std::string                            name
) {
    for (const std::string &descriptor : DESCRIPTOR_TYPE_MAP[t_type])
        if (descriptor == name) return true;
    return false;
}

VkDescriptorSetLayout_T *IE::Graphics::DescriptorSet::getLayout(
  IE::Graphics::RenderEngine          *t_engineLink,
  size_t                               t_set,
  std::vector<std::shared_ptr<Shader>> t_shaders
) {
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
    for (std::shared_ptr<Shader> &shader : t_shaders) {
        for (Shader::ReflectionInfo &info : shader->reflectedInfo) {
            if (info.set == t_set) {
                layoutBindings.push_back(
                  {.binding            = info.binding,
                   .descriptorType     = info.type,
                   .descriptorCount    = 1,  // Indicated size of array in shader. Arrays are not supported yet.
                   .stageFlags         = shader->m_stage,
                   .pImmutableSamplers = nullptr}
                );
            }
        }
    }
    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{
      .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext        = nullptr,
      .flags        = 0x0,
      .bindingCount = static_cast<uint32_t>(layoutBindings.size()),
      .pBindings    = layoutBindings.data()};
    VkDescriptorSetLayout layout;
    vkCreateDescriptorSetLayout(t_engineLink->m_device.device, &layoutCreateInfo, nullptr, &layout);
    return layout;
}

IE::Graphics::DescriptorSet::DescriptorSet(IE::Graphics::DescriptorSet::SetNumber t_type) : m_setNumber(t_type) {
}

const std::array<std::vector<std::string>, 4> IE::Graphics::DescriptorSet::DESCRIPTOR_TYPE_MAP{
  IE::Graphics::DescriptorSet::PER_FRAME_DESCRIPTORS,
  IE::Graphics::DescriptorSet::PER_SUBPASS_DESCRIPTORS,
  IE::Graphics::DescriptorSet::PER_MATERIAL_DESCRIPTORS,
  IE::Graphics::DescriptorSet::PER_OBJECT_DESCRIPTORS};
const std::vector<std::string> IE::Graphics::DescriptorSet::PER_FRAME_DESCRIPTORS{"camera"};
const std::vector<std::string> IE::Graphics::DescriptorSet::PER_SUBPASS_DESCRIPTORS{"perspective"};
const std::vector<std::string> IE::Graphics::DescriptorSet::PER_MATERIAL_DESCRIPTORS{
  "diffuseTexture",
  "specularTexture"};

const std::vector<std::string> IE::Graphics::DescriptorSet::PER_OBJECT_DESCRIPTORS{"object"};

const std::array<std::vector<VkDescriptorPoolSize>, 4> IE::Graphics::DescriptorSet::DESCRIPTOR_POOL_SIZES{
  {{{.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 2}},
   {{.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1}},
   {{.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, .descriptorCount = 1}},
   {{.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1}}}
};

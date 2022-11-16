#include "DescriptorSet.hpp"

#include "GraphicsModule/RenderPass/RenderPass.hpp"
#include "GraphicsModule/RenderPass/RenderPassSeries.hpp"
#include "GraphicsModule/RenderPass/Subpass.hpp"
#include "RenderEngine.hpp"

#include <array>
#include <vector>

void IE::Graphics::DescriptorSet::build(
  IE::Graphics::Subpass             *t_subpass,
  std::vector<IE::Graphics::Shader> &t_shaders
) {
    m_subpass = t_subpass;

    //    for (Shader shader : t_shaders) {
    //        spv_reflect::ShaderModule     &module = shader.reflect();
    //        SpvReflectResult               result;
    //        const SpvReflectDescriptorSet *set{module.GetDescriptorSet(m_type, &result)};
    //        for (size_t i{}; result == SPV_REFLECT_RESULT_SUCCESS && i < set->binding_count; ++i) {
    //            bool found{};
    //            for (auto &poolSize : poolSizes) {
    //                if (poolSize.type == static_cast<VkDescriptorType>(set->bindings[i]->descriptor_type)) {
    //                    poolSize.descriptorCount += set->bindings[i]->count;
    //                    found = true;
    //                    break;
    //                }
    //            }
    //            if (!found)
    //                poolSizes.push_back(
    //                  {.type            = static_cast<VkDescriptorType>(set->bindings[i]->descriptor_type),
    //                   .descriptorCount = set->bindings[i]->count}
    //                );
    //        }
    //    }

    VkDescriptorPoolCreateInfo poolCreateInfo{
      .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .pNext         = nullptr,
      .flags         = 0x0,
      .maxSets       = 0x1,
      .poolSizeCount = static_cast<uint32_t>(DESCRIPTOR_POOL_SIZES[m_type].size()),
      .pPoolSizes    = DESCRIPTOR_POOL_SIZES[m_type].data()};

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
      .poolSizeCount = static_cast<uint32_t>(DESCRIPTOR_POOL_SIZES[m_type].size()),
      .pPoolSizes    = DESCRIPTOR_POOL_SIZES[m_type].data()};

    VkResult result{vkCreateDescriptorPool(t_engineLink->m_device.device, &poolCreateInfo, nullptr, &m_pool)};
    if (result != VK_SUCCESS)
        t_engineLink->getLogger().log(
          "Failed to create descriptor pool with error: " +
          IE::Graphics::RenderEngine::translateVkResultCodes(result)
        );
    else t_engineLink->getLogger().log("Created Descriptor Pool");
}

bool IE::Graphics::DescriptorSet::isDescriptorControlledBySetType(
  IE::Graphics::DescriptorSet::Type t_type,
  std::string                       name
) {
    for (const std::string &descriptor : DESCRIPTOR_TYPE_MAP[t_type])
        if (descriptor == name) return true;
    return false;
}

IE::Graphics::DescriptorSet::DescriptorSet(IE::Graphics::DescriptorSet::Type t_type) : m_type(t_type) {
}

const std::array<std::vector<std::string>, 4> IE::Graphics::DescriptorSet::DESCRIPTOR_TYPE_MAP{
  IE::Graphics::DescriptorSet::PER_FRAME_DESCRIPTORS,
  IE::Graphics::DescriptorSet::PER_SUBPASS_DESCRIPTORS,
  IE::Graphics::DescriptorSet::PER_MATERIAL_DESCRIPTORS,
  IE::Graphics::DescriptorSet::PER_OBJECT_DESCRIPTORS};

const std::vector<std::string> IE::Graphics::DescriptorSet::PER_FRAME_DESCRIPTORS{"camera"};
const std::vector<std::string> IE::Graphics::DescriptorSet::PER_SUBPASS_DESCRIPTORS{"perspective"};
const std::vector<std::string> IE::Graphics::DescriptorSet::PER_MATERIAL_DESCRIPTORS{"diffuseTexture"};
const std::vector<std::string> IE::Graphics::DescriptorSet::PER_OBJECT_DESCRIPTORS{""};

const std::array<std::vector<VkDescriptorPoolSize>, 4> IE::Graphics::DescriptorSet::DESCRIPTOR_POOL_SIZES{
  {{{.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 2}},
   {{.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1}},
   {{.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, .descriptorCount = 1}},
   {{.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1}}}
};

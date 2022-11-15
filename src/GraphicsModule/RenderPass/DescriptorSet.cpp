#include "DescriptorSet.hpp"

#include "RenderEngine.hpp"
#include "RenderPass.hpp"
#include "RenderPassSeries.hpp"
#include "Subpass.hpp"

struct DescriptorSetLayoutData {
    VkDescriptorSetLayoutCreateInfo           create_info;
    std::vector<VkDescriptorSetLayoutBinding> bindings;
} __attribute__((aligned(64)));

void IE::Graphics::DescriptorSet::build(
  IE::Graphics::Subpass             *t_subpass,
  std::vector<IE::Graphics::Shader> &t_shaders
) {
    m_subpass = t_subpass;

    for (auto &shader : t_shaders) {
        spv_reflect::ShaderModule module{shader.reflect()};
        uint32_t                  count{};
        SpvReflectResult          result{module.EnumerateDescriptorSets(&count, nullptr)};

        std::vector<SpvReflectDescriptorSet *> sets(count);
        result = module.EnumerateDescriptorSets(&count, sets.data());

        std::vector<DescriptorSetLayoutData> set_layouts(sets.size(), DescriptorSetLayoutData{});
        for (size_t i{0}; i < sets.size(); ++i) {
            const SpvReflectDescriptorSet &set    = *(sets[i]);
            DescriptorSetLayoutData       &layout = set_layouts[i];
            layout.bindings.resize(set.binding_count);
            for (uint32_t j{0}; j < set.binding_count; ++j) {
                const SpvReflectDescriptorBinding &binding        = *(set.bindings[j]);
                VkDescriptorSetLayoutBinding      &layout_binding = layout.bindings[j];
                layout_binding.binding                            = binding.binding;
                layout_binding.descriptorType  = static_cast<VkDescriptorType>(binding.descriptor_type);
                layout_binding.descriptorCount = 1;
                for (uint32_t k{0}; k < binding.array.dims_count; ++k)
                    layout_binding.descriptorCount *= binding.array.dims[k];
                layout_binding.stageFlags = static_cast<VkShaderStageFlagBits>(module.GetShaderStage());
            }
            layout.create_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layout.create_info.bindingCount = set.binding_count;
            layout.create_info.pBindings    = layout.bindings.data();
        }
    }

    //    vkCreateDescriptorSetLayout();
    //    VkDescriptorPoolCreateInfo poolCreateInfo{
    //      .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    //      .pNext         = nullptr,
    //      .flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
    //      .maxSets       = 0x1,
    //      .poolSizeCount = };
    //
    //    vkCreateDescriptorPool(m_subpass->m_renderPass->m_renderPassSeries->m_linkedRenderEngine->m_device.device,
    //    );
}

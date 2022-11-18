#pragma once

#include "DescriptorSet/DescriptorSet.hpp"

namespace IE::Graphics {
class Renderable {
    DescriptorSet descriptorSet{DescriptorSet::IE_DESCRIPTOR_SET_TYPE_PER_OBJECT};
};
}  // namespace IE::Graphics
/* Include this file's header. */
#include "IEVertex.hpp"

/* Include external dependencies. */
#include <vulkan/vulkan.h>
#include <string>

VkVertexInputBindingDescription IEVertex::getBindingDescription() {
	return {
			.binding=0,
			.stride=sizeof(IEVertex),
			.inputRate=VK_VERTEX_INPUT_RATE_VERTEX
	};
}

std::array<VkVertexInputAttributeDescription, 6> IEVertex::getAttributeDescriptions() {
	return {
			{
					{
							.location = 0,
							.binding = 0,
							.format = VK_FORMAT_R32G32B32_SFLOAT,
							.offset = offsetof(IEVertex, position),
					},
					{
							.location = 1,
							.binding = 0,
							.format = VK_FORMAT_R32G32B32_SFLOAT,
							.offset = offsetof(IEVertex, color),
					},
					{
							.location = 2,
							.binding = 0,
							.format = VK_FORMAT_R32G32_SFLOAT,
							.offset = offsetof(IEVertex, textureCoordinates),
					},
					{
							.location = 3,
							.binding = 0,
							.format = VK_FORMAT_R32G32B32_SFLOAT,
							.offset = offsetof(IEVertex, normal),
					},
					{
							.location = 4,
							.binding = 0,
							.format = VK_FORMAT_R32G32B32_SFLOAT,
							.offset = offsetof(IEVertex, tangent),
					},
					{
							.location = 5,
							.binding = 0,
							.format = VK_FORMAT_R32G32B32_SFLOAT,
							.offset = offsetof(IEVertex, biTangent),
					}
			}
	};
}

std::string IEVertex::toString() {
	return "Position: x: " + std::to_string(position.x) + ",\ty: " + std::to_string(position.y) + ",\tz: " + std::to_string(position.z);
}

bool IEVertex::operator==(IEVertex &other) {
	return position == other.position && color == other.color && textureCoordinates == other.textureCoordinates && normal == other.normal && tangent == other.tangent && biTangent == other.biTangent;
}

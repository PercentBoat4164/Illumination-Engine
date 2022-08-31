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

void IEVertex::useVertexAttributesWithProgram(GLint program) {
	int attributeLocation = glGetAttribLocation(program, "vertexPosition");
	if (attributeLocation >= 0) {
		glEnableVertexAttribArray(attributeLocation);
		glVertexAttribPointer(attributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(IEVertex), (void *) offsetof(IEVertex, position));
	}
	attributeLocation = glGetAttribLocation(program, "vertexColor");
	if (attributeLocation >= 0) {
		glEnableVertexAttribArray(attributeLocation);
		glVertexAttribPointer(attributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(IEVertex), (void *) offsetof(IEVertex, color));
	}
	attributeLocation = glGetAttribLocation(program, "vertexTextureCoordinates");
	if (attributeLocation >= 0) {
		glEnableVertexAttribArray(attributeLocation);
		glVertexAttribPointer(attributeLocation, 2, GL_FLOAT, GL_FALSE, sizeof(IEVertex), (void *) offsetof(IEVertex, textureCoordinates));
	}
	attributeLocation = glGetAttribLocation(program, "vertexNormal");
	if (attributeLocation >= 0) {
		glEnableVertexAttribArray(attributeLocation);
		glVertexAttribPointer(attributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(IEVertex), (void *) offsetof(IEVertex, normal));
	}
	attributeLocation = glGetAttribLocation(program, "vertexTangent");
	if (attributeLocation >= 0) {
		glEnableVertexAttribArray(attributeLocation);
		glVertexAttribPointer(attributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(IEVertex), (void *) offsetof(IEVertex, tangent));
	}
	attributeLocation = glGetAttribLocation(program, "vertexBiTangent");
	if (attributeLocation >= 0) {
		glEnableVertexAttribArray(attributeLocation);
		glVertexAttribPointer(attributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(IEVertex), (void *) offsetof(IEVertex, biTangent));
	}
}

bool IEVertex::operator==(IEVertex &other) const {
	return position == other.position && color == other.color && textureCoordinates == other.textureCoordinates && normal == other.normal && tangent == other.tangent && biTangent == other.biTangent;
}

/* Include this file's header. */
#include "Vertex.hpp"

/* Include external dependencies. */
#include <string>
#include <vulkan/vulkan.h>

VkVertexInputBindingDescription Vertex::getBindingDescription() {
    return {.binding = 0, .stride = sizeof(Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};
}

std::array<VkVertexInputAttributeDescription, 6> Vertex::getAttributeDescriptions() {
    return {
      {{
         .location = 0,
         .binding  = 0,
         .format   = VK_FORMAT_R32G32B32_SFLOAT,
         .offset   = offsetof(Vertex, position),
       }, {
         .location = 1,
         .binding  = 0,
         .format   = VK_FORMAT_R32G32B32_SFLOAT,
         .offset   = offsetof(Vertex, color),
       }, {
         .location = 2,
         .binding  = 0,
         .format   = VK_FORMAT_R32G32_SFLOAT,
         .offset   = offsetof(Vertex, textureCoordinates),
       }, {
         .location = 3,
         .binding  = 0,
         .format   = VK_FORMAT_R32G32B32_SFLOAT,
         .offset   = offsetof(Vertex, normal),
       }, {
         .location = 4,
         .binding  = 0,
         .format   = VK_FORMAT_R32G32B32_SFLOAT,
         .offset   = offsetof(Vertex, tangent),
       }, {
         .location = 5,
         .binding  = 0,
         .format   = VK_FORMAT_R32G32B32_SFLOAT,
         .offset   = offsetof(Vertex, biTangent),
       }}
    };
}

void Vertex::useVertexAttributesWithProgram(GLint program) {
    int attributeLocation = glGetAttribLocation(program, "vertexPosition");
    if (attributeLocation >= 0) {
        glEnableVertexAttribArray(attributeLocation);
        glVertexAttribPointer(
          attributeLocation,
          3,
          GL_FLOAT,
          GL_FALSE,
          sizeof(Vertex),
          (void *) offsetof(Vertex, position)
        );
    }
    attributeLocation = glGetAttribLocation(program, "vertexColor");
    if (attributeLocation >= 0) {
        glEnableVertexAttribArray(attributeLocation);
        glVertexAttribPointer(
          attributeLocation,
          4,
          GL_FLOAT,
          GL_FALSE,
          sizeof(Vertex),
          (void *) offsetof(Vertex, color)
        );
    }
    attributeLocation = glGetAttribLocation(program, "vertexTextureCoordinates");
    if (attributeLocation >= 0) {
        glEnableVertexAttribArray(attributeLocation);
        glVertexAttribPointer(
          attributeLocation,
          2,
          GL_FLOAT,
          GL_FALSE,
          sizeof(Vertex),
          (void *) offsetof(Vertex, textureCoordinates)
        );
    }
    attributeLocation = glGetAttribLocation(program, "vertexNormal");
    if (attributeLocation >= 0) {
        glEnableVertexAttribArray(attributeLocation);
        glVertexAttribPointer(
          attributeLocation,
          3,
          GL_FLOAT,
          GL_FALSE,
          sizeof(Vertex),
          (void *) offsetof(Vertex, normal)
        );
    }
    attributeLocation = glGetAttribLocation(program, "vertexTangent");
    if (attributeLocation >= 0) {
        glEnableVertexAttribArray(attributeLocation);
        glVertexAttribPointer(
          attributeLocation,
          3,
          GL_FLOAT,
          GL_FALSE,
          sizeof(Vertex),
          (void *) offsetof(Vertex, tangent)
        );
    }
    attributeLocation = glGetAttribLocation(program, "vertexBiTangent");
    if (attributeLocation >= 0) {
        glEnableVertexAttribArray(attributeLocation);
        glVertexAttribPointer(
          attributeLocation,
          3,
          GL_FLOAT,
          GL_FALSE,
          sizeof(Vertex),
          (void *) offsetof(Vertex, biTangent)
        );
    }
}

bool Vertex::operator==(Vertex &other) const {
    return position == other.position && color == other.color && textureCoordinates == other.textureCoordinates &&
      normal == other.normal && tangent == other.tangent && biTangent == other.biTangent;
}

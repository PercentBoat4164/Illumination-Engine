#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtc/quaternion.hpp>

struct OpenGLVertex {
public:
    glm::vec3 pos{};
    glm::vec4 color{};
    glm::vec2 texCoord{};
    glm::vec3 normal{};

    bool operator==(const OpenGLVertex &other) const { return pos == other.pos && color == other.color && texCoord == other.texCoord; }
};

template<> struct std::hash<OpenGLVertex> { size_t operator()(OpenGLVertex const& vertex) const { return hash<glm::vec3>()(vertex.pos); } };
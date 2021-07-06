#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtc/quaternion.hpp>

struct OpenGLVertex {
public:
    glm::vec3 pos{0.0f, 0.0f, 0.0f};
    glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec2 texCoords{0.0f,0.0f};
    glm::vec3 normal{0.5f, 0.5f, 0.5f};

    bool operator==(const OpenGLVertex &other) const { return pos == other.pos && color == other.color && texCoords == other.texCoords && normal == other.normal; }
};

template<> struct std::hash<OpenGLVertex> { size_t operator()(OpenGLVertex const& vertex) const { return hash<glm::vec3>()(vertex.pos); } };
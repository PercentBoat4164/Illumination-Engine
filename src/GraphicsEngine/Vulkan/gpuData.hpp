#pragma once

#include <glm/glm.hpp>

/** This is the structure for the UniformBufferObject.*/
struct UniformBufferObject {
public:
    /** This is a matrix4 variable called model{}.*/
    alignas(16) glm::mat4 model{};
    /** This is a matrix4 variable called view{}.*/
    alignas(16) glm::mat4 view{};
    /** This is a matrix4 variable called proj{}.*/
    alignas(16) glm::mat4 proj{};
};
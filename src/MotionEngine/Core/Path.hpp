#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"

//a Path that an object can follow
class Path {
public:

    glm::vec3 *position;
    glm::quat *rotation;

    //translates and rotates an object relative to time
    virtual void step(float time) = 0;
};
#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"

//a Route that an object can follow
class Route {
public:

    glm::vec3 *position{};
    glm::quat *rotation{};

    //translates and rotates an object relative to time
    virtual void step(float time) = 0;
};
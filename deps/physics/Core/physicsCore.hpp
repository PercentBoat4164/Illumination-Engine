#pragma once

#include "../../glm/glm.hpp"
#include "Geometry.hpp"
#include <list>

struct Force {
    glm::vec3 force;
    glm::vec3 * reference;
    float startTime, endTime;
};

class Body : Shape {
    float mass;
    glm::vec3 velocity;
    glm::vec3 angularVelocity;
    list <Force> :: iterator it;
};
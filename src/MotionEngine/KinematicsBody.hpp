#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"

class KinematicsBody {
public:
    float mass; //mass of the body
    glm::quat rotationalVelocity; //rad/s
    glm::vec3 velocity;
    glm::vec3 acceleration;
    glm::vec3 position;
    glm::quat rotation;

    KinematicsBody(float m, glm::vec3 v, glm::quat rotV, glm::vec3 a) {
        mass = m;
        velocity = v;
        rotationalVelocity = rotV;
        acceleration = a;
    }
};
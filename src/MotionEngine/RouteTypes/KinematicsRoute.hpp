#pragma once

#include "../Core/Route.hpp"
#include "../KinematicsBody.hpp"
//a class that moves objects along a predicted physics path
class KinematicsRoute: public Route {
public:
    float *mass{}; //mass of the body
    glm::quat *rotationalVelocity{}; //rad/s
    glm::vec3 *velocity{};
    glm::vec3 *acceleration{};

    void step(float time) {
        *position += *velocity * time + *acceleration * time * time * 0.5f;
        *velocity += *acceleration * time;
        *rotation = glm::slerp(*rotation, *rotationalVelocity, time);
    }

    void applyImpulse(glm::vec3 impulse) {
        *velocity += impulse;
    }

    void setBody(KinematicsBody* body) {
        mass = &body->mass;
        rotationalVelocity = &body->rotationalVelocity;
        velocity = &body->velocity;
        acceleration = &body->acceleration;
        position = &body->position;
        rotation = &body->rotation;
    }
};
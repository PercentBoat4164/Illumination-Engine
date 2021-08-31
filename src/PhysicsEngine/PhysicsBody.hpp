#pragma once

#include "PhysicsMesh.hpp"
#include <glm/glm.hpp>

//A physics body
class PhysicsBody {
public:

    float mass{0}; //mass of the body
    glm::vec3 position{glm::vec3(0,0,0)}; //the position (and center of mass) of the body
    glm::vec3 velocity{glm::vec3(0,0,0)}; //the velocity of the body
    PhysicsMesh mesh{};
    
    //step the body forward in time
    void step(float time) {
        position += velocity;
    }

    //apply an impulse to accelerate the body
    void applyImpulse(glm::vec3 impulse) {
        velocity += impulse;
    }
};
#pragma once
#include "Geometry.hpp"
#include <glm/glm.hpp>

class Body {
public:

    float mass{0}; //mass of the body
    glm::vec3 position{glm::vec3(0,0,0)}; //the position of the body
    glm::vec3 centerOfMass{glm::vec3(0,0,0)}; //the CoM of the body
    glm::vec3 velocity{glm::vec3(0,0,0)}; //the velocity of the body
    Shape *shape;
    
    //step the body forward in time
    void step(float time) {
        position += velocity;
        shape -> position = position;
    }

    //apply an impulse to accelerate the body
    void applyImpulse(glm::vec3 impulse) {
        velocity += impulse;
    }


};

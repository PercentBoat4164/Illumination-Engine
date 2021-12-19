#pragma once

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif
#include "PhysicsMesh.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"

//A physics body
class PhysicsBody {
public:

    float mass{0}; //mass of the body
    glm::vec3 position{glm::vec3(0,0,0)}; //the position (and center of mass) of the body
    glm::quat rotation{1,0,0,0}; //the current heading of the body
    glm::quat rotationalVelocity{1,0,0,0}; //the rotational velocity
    glm::vec3 velocity{glm::vec3(0,0,0)}; //the velocity of the body
    
    //step the body forward in time
    void step(float time) {

        //update xyz position 
        position += velocity;

        rotation += glm::pow(rotationalVelocity, time);
    }

    //apply an impulse to accelerate the body
    void applyImpulse(glm::vec3 impulse) {
        velocity += impulse;
    }

    //get the current location of a vertex in the mesh
    glm::vec3 getMeshVertex(int index) {
        return position + glm::rotate(rotation, mesh.vertices[index]);
    }

    std::vector<glm::vec3> getMeshTri(int index) {

        //the triangle to be returned
        std::vector<glm::vec3> tri = mesh.getTri(index);
        
        //move each vec3 in the tri to its current location in the physics world
        for(int i = 0; i < 3; i++) {
            tri[i] = glm::rotate(rotation, tri[i]);
            tri[i] += position;
        }
        return tri;
    }

    int getMeshSize() {
        return mesh.indices.size();
    }

private:
    PhysicsMesh mesh{};
};
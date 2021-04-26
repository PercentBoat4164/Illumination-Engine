#pragma once

#include "../../glm/glm.hpp"
#include "Geometry.hpp"
#include <list>

class Body {
    public:
        float mass; //mass of the body
        
        void step(float time);

        void applyImpulse(glm::vec3 impulse);

        glm::vec3 getPosition();

        void setPosition(glm::vec3 newPosition);

        bool isIntersecting(Body otherBody);

        Shape getShape();

        glm::vec3 getVelocity();

        void setVelocity(glm::vec3 newVelocity);

    private:

        Shape myShape;
        glm::vec3 centerOfMass; //the center of mass of the body
        glm::vec3 velocity; //the velocity of the body
};

class World {
    public:

        //add a body to the world
        void addBody(Body *body);
        
        //add a body to the world and list of active bodies
        void addActiveBody(Body *body);

        //step the world forward in time
        void step(float time);

        //remove a body from the world
        void removeBody(Body *body);

        //returns whether the bodies collide within the described time
        bool checkCollision(Body body1, Body body2, float time);

        //get the exact time of a collision between two bodies
        float getCollisionTime(Body body1, Body body2);
        
    private:
    
        //every body in the world
        std::list<Body *> bodies;

        //bodies that are not at rest
        std::list<Body *> activeBodies;

        //add a body in the world to the list of active bodies
        void activateBody(Body *body);

        //remove a body in the world from the list of active bodies
        void deactivateBody(Body *body);
};
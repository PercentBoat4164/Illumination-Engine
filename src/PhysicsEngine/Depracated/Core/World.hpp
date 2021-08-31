#pragma once

#include <glm/glm.hpp>
#include "Geometry.hpp"
#include "Body.hpp"
#include <list>

class World {
    public:

        //add a body to the world
        void addBody(Body *body) {
            bodies.push_back(body);
        }
        
        //add a body to the world and list of active bodies
        void addActiveBody(Body *body) {
            activeBodies.push_back(body);
            bodies.push_back(body);
        }

        //step the world forward in time
        void step(float time) {
            for(Body * const& i : activeBodies) {
                i->step(time);
            }
        }

        //remove a body from the world
        void removeBody(Body *body) {
            bodies.remove(body);
        }
        
        //returns whether the bodies collide within the described time
        bool checkCollision(Body body1, Body body2, float time) {
            body1.step(time);
            body2.step(time);
            return glm::length(body1.shape->position - body2.shape->position) <= body1.shape->radius + body2.shape->radius;
        }

        //get the exact time of a collision between two bodies
        float getCollisionTime(Body body1, Body body2) {
            return 0;
        }

        void semiElasticCollide(Body *body1, Body *body2) {
            Body s1 = *body1;
            Body s2 = *body2;
            
            //get the line on which the collision takes place. The method calculates a 1d collision along this line
            glm::vec3 collisionNormal = s1.position - s2.position;
            collisionNormal *= 1 / glm::length(collisionNormal);

            glm::vec3 vs1 = collisionNormal * glm::dot(s1.velocity, collisionNormal);
            glm::vec3 vs2 = collisionNormal * glm::dot(s2.velocity, collisionNormal);

            glm::vec3 remainderS1 = s1.velocity - vs1;
            glm::vec3 remainderS2 = s2.velocity - vs2;

            glm::vec3 vS1Result = (s1.mass-s2.mass)/(s1.mass+s2.mass)*vs1 + (2*s2.mass)/(s1.mass+s2.mass)*vs2;
            glm::vec3 vS2Result = (s2.mass-s1.mass)/(s2.mass+s1.mass)*vs2 + (2*s1.mass)/(s2.mass+s1.mass)*vs1;

            //Recombine the velocity
            body1->velocity = (vS1Result + remainderS1);
            body2->velocity = (vS2Result + remainderS2);
        }

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
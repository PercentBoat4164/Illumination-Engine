#include "PhysicsCore.hpp"

class Body {
    public:

        Body(Shape shape, float setMass) {
            myShape = shape;
            mass = setMass;
            centerOfMass = shape.position;
            velocity = glm::vec3(0, 0, 0);
        }

        float mass; //mass of the body
        
        void step(float time) {
            myShape.position += velocity * time;
        }

        void applyImpulse(glm::vec3 impulse) {
            velocity += impulse;
        }

        glm::vec3 getPosition() {
            return myShape.position;
        }

        void setPosition(glm::vec3 newPosition) {
            myShape.position = newPosition;
        }

        bool isIntersecting(Body otherBody) {
            return myShape.isIntersecting(otherBody.getShape());
        }

        Shape getShape() {
            return myShape;
        }

        glm::vec3 getVelocity() {
            return velocity;
        }

        void setVelocity(glm::vec3 newVelocity) {
            velocity = newVelocity;
        }

    private:

        Shape myShape;
        glm::vec3 centerOfMass; //the center of mass of the body
        glm::vec3 velocity; //the velocity of the body
};

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
            return body1.isIntersecting(body2);
        }

        //get the exact time of a collision between two bodies
        float getCollisionTime(Body body1, Body body2) {
            return 0;
        }

        void semiElasticCollide(Body *body1, Body *body2) {
            Body s1 = *body1;
            Body s2 = *body2;
            
            //get the line on which the collision takes place. The method calculates a 1d collision along this line
            glm::vec3 collisionNormal = s1.getPosition() - s2.getPosition();
            collisionNormal *= 1 / glm::length(collisionNormal);

            glm::vec3 vs1 = collisionNormal * glm::dot(s1.getVelocity(), collisionNormal);
            glm::vec3 vs2 = collisionNormal * glm::dot(s2.getVelocity(), collisionNormal);

            glm::vec3 remainderS1 = s1.getVelocity() - vs1;
            glm::vec3 remainderS2 = s2.getVelocity() - vs2;

            glm::vec3 vS1Result = (s1.mass-s2.mass)/(s1.mass+s2.mass)*vs1 + (2*s2.mass)/(s1.mass+s2.mass)*vs2;
            glm::vec3 vS2Result = (s2.mass-s1.mass)/(s2.mass+s1.mass)*vs2 + (2*s1.mass)/(s2.mass+s1.mass)*vs1;

            //Recombine the velocity
            s1.setVelocity(vS1Result + remainderS1);
            s1.setVelocity(vS2Result + remainderS2);
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
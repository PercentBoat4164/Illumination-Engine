#define TINYOBJLOADER_IMPLEMENTATION
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

#include "sources/tiny_obj_loader.h"


//A particle with a position, velocity, and mass.
class Particle {
public:

    glm::vec4 pos; //position
    glm::vec4 v; //velocity
    float m; //mass

    //constructor
    Particle(float x, float y, float z, float mass) {
        pos = glm::vec4(x, y, z, 1.0f);
        v = glm::vec4(0.0f,0.0f,0.0f, 0.0f);
        m = mass;
    }

    //step forward in time. Should be called every tick
    void step() {
        pos += v;
    }

    //apply an impulse to the body
    void applyImpulse(glm::vec4 impulse) {
        v += impulse;
    }

    void applyImpulse(float x, float y, float z) {
        v.x += (x/m);
        v.y += (y/m);
        v.z += (z/m);
    }
protected:
    Particle() {
        pos = glm::vec4(0.0f,0.0f,0.0f,0.0f);
        v = glm::vec4(0.0f,0.0f,0.0f,0.0f);
        m = 0.0f;
    }
};

class RigidBody: public Particle {
public:
    glm::vec3 rot; //the current angular position of the rigidbody
    glm::vec3 rotV; //the current angular velocity of the rigidbody
    float I; //the moment of inertia for the rigidbody

    RigidBody(float x,float y,float z,float m) : Particle(x,y,z,m) {
        rot = glm::vec3();
        rotV = glm::vec3();
    }


protected:
    RigidBody() {

    }
};

class SphereBody: public RigidBody {
public:


    float r; //radius

    SphereBody(float x,float y,float z,float mass,float radius) {
        pos = glm::vec4(x,y,z,0.0f);
        m=mass;
        r=radius;
    }

    bool checkCollision(SphereBody otherSphere) {
        if(r + otherSphere.r > glm::length(otherSphere.pos - pos)) {
            return true;
        }
        return false;
    }
    void bounce(SphereBody otherSphere) {

    }
};

//creates a world
class World {
public:

    std::vector<Particle*> bodies;

    void addBody(Particle *body) {
        bodies.push_back(body);
    }

    void step() {
        for(auto & body : bodies) {
            body -> step();
        }
    }
};
int main() {
    SphereBody myBody = SphereBody(0.0f, 0.0f, 0.0f, 1.0f,2.0f);
    SphereBody otherBody = SphereBody(5.0f, 0.0f, 0.0f, 1.0f,2.0f);
    World myWorld;
    myWorld.addBody(&myBody);
    char input;
    do {
        std::cin >> input;
        switch(input) {
            case 'p':
                myWorld.step();
                break;
            case 'w':
                myBody.applyImpulse(0,1,0);
                break;
            case 'a':
                myBody.applyImpulse(-1,0,0);
                break;
            case 's':
                myBody.applyImpulse(0,-1,0);
                break;
            case 'd':
                myBody.applyImpulse(1,0,0);
                break;
            case 'z':
                myBody.applyImpulse(0,0,1);
                break;
            case 'x':
                myBody.applyImpulse(0,0,-1);
                break;

        }
        std::cout << "\nVx: " << myBody.v.x << "  Vy: " << myBody.v.y << "  Vz: " << myBody.v.z;
        std::cout << "\nPos x: " << myBody.pos.x << " Pos y: " << myBody.pos.y << " Pos z: " << myBody.pos.z << "\n";
        if(myBody.checkCollision(otherBody)) {
            std::cout << "Intersecting\n";
        } else {
            std::cout << "Not intersecting\n";
        }
    } while(input != 'e');
    return 0;
}
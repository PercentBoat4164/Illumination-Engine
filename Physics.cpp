#define TINYOBJLOADER_IMPLEMENTATION
#include <glm/glm.hpp>
#include <glm/gtx/component_wise.hpp>
#include <iostream>
#include <vector>

#include "sources/tiny_obj_loader.h"

float getLineDist(glm::vec3 p1,glm::vec3 p2,glm::vec3 p3,glm::vec3 p4) {
    glm::vec3 pa, pb, p13, p43, p21;
    float epsilon = 0.01;
    p13 = p1 - p3;
    p43 = p4 - p3;
    if (std::abs(p43.x) < epsilon && std::abs(p43.y) < epsilon && std::abs(p43.z) < epsilon) {return -1.0f;}
    p21 = p2 - p3;
    if (std::abs(p21.x) < epsilon && std::abs(p21.y) < epsilon && std::abs(p21.z) < epsilon) {return -2.0f;}
    float d1343 = glm::compAdd(p13 * p43);
    float d4321 = glm::compAdd(p43 * p21);
    float d4343 = glm::compAdd(p43 * p43);
    float denom = glm::compAdd(p21 * p21) * d4343 - d4321 * d4321;
    float numer = d1343 * d4321 - glm::compAdd(p13 * p21) * d4343;
    if (std::abs(denom) < epsilon) {return -3.0f;} //Are lines parallel?
    float mua = numer / denom;
    float mub = (d1343 + d4321 * (mua)) / d4343;
    pa = p1 + mua * p21;
    pb = p3 + mub * p43;
    return(glm::length(pa-pb));
}

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

    //replaced by method in world?
    /*bool checkCollision(SphereBody otherSphere) {
        if(r + otherSphere.r > glm::length(otherSphere.pos - pos)) {
            return true;
        }
        return false;
    }*/
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

    bool checkCollision(SphereBody s1, SphereBody s2) {

        float l = getLineDist(s1.pos,(s1.pos+s1.v),s2.pos,(s2.pos+s2.v));
        std::cout << "Distance between lines: " << l << "\n";
        if(l <= 0) {
            l = s1.r + s2.r+1;
        }
        if(s1.r + s2.r > glm::length(s2.pos - s1.pos) || l < s1.r + s2.r) {
            return true;
        }
        return false;
    }
};
int main() {
    SphereBody myBody = SphereBody(0.0f, 0.0f, 0.0f, 1.0f,2.0f);
    SphereBody otherBody = SphereBody(5.0f, 1.0f, 1.0f, 1.0f,2.0f);
    otherBody.applyImpulse(0,0,1);
    World myWorld;
    myWorld.addBody(&myBody);
    myWorld.addBody(&otherBody);
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
        std::cout << "-----myBody--------";
        std::cout << "\nVx: " << myBody.v.x << "  Vy: " << myBody.v.y << "  Vz: " << myBody.v.z;
        std::cout << "\nPos x: " << myBody.pos.x << " Pos y: " << myBody.pos.y << " Pos z: " << myBody.pos.z << "\n";
        std::cout << "-----otherBody-----";
        std::cout << "\nVx: " << otherBody.v.x << "  Vy: " << otherBody.v.y << "  Vz: " << otherBody.v.z;
        std::cout << "\nPos x: " << otherBody.pos.x << " Pos y: " << otherBody.pos.y << " Pos z: " << otherBody.pos.z << "\n" << std::endl;
        if(myWorld.checkCollision(myBody, otherBody)) {
            std::cout << "Intersecting\n";
        } else {
            std::cout << "Not intersecting\n";
        }
    } while(input != 'e');
    return 0;
}
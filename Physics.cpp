#define TINYOBJLOADER_IMPLEMENTATION
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

#include "sources/tiny_obj_loader.h"

float getLineDist(glm::vec3 p1,glm::vec3 p2,glm::vec3 p3,glm::vec3 p4) {

    glm::vec3 pa,pb,p13,p43,p21;
    float d1343,d4321,d1321,d4343,d2121;
    float mua,mub,numer,denom;
    float epsilon = 0.01;

    p13 = p1-p3;
    p43 = p4-p3;
    std::cout <<"\nPoints: p1x: " << p1.x << "  p3x: " << p3.x << "  p4x: " <<p4.x <<"\n";
    if (std::abs(p43.x) < epsilon && std::abs(p43.y) < epsilon && std::abs(p43.z) < epsilon) {
        return -1.0f;
    }
    p21 = p2-p3;
    if (std::abs(p21.x) < epsilon && std::abs(p21.y) < epsilon && std::abs(p21.z) < epsilon) {
        return -2.0f;
    }

    d1343 = p13.x * p43.x + p13.y * p43.y + p13.z * p43.z;
    d4321 = p43.x * p21.x + p43.y * p21.y + p43.z * p21.z;
    d1321 = p13.x * p21.x + p13.y * p21.y + p13.z * p21.z;
    d4343 = p43.x * p43.x + p43.y * p43.y + p43.z * p43.z;
    d2121 = p21.x * p21.x + p21.y * p21.y + p21.z * p21.z;

    denom = d2121 * d4343 - d4321 * d4321;
    if (std::abs(denom) < epsilon) {
        return -3.0f;
    }
    numer = d1343 * d4321 - d1321 * d4343;

    mua = numer / denom;
    mub = (d1343 + d4321 * (mua)) / d4343;

    pa.x = p1.x + mua * p21.x;
    pa.y = p1.y + mua * p21.y;
    pa.z = p1.z + mua * p21.z;
    pb.x = p3.x + mub * p43.x;
    pb.y = p3.y + mub * p43.y;
    pb.z = p3.z + mub * p43.z;

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
            l = s1.r+s2.r+1;
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
        if(myWorld.checkCollision(myBody,otherBody)) {
            std::cout << "Intersecting\n";
        } else {
            std::cout << "Not intersecting\n";
        }
    } while(input != 'e');
    return 0;
}
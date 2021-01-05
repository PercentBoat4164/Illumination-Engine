#define TINYOBJLOADER_IMPLEMENTATION
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include "sources/tiny_obj_loader.h"

//returns the distance between two lines in 3d space
float distLineLine(glm::vec3 pos1, glm::vec3 v1, glm::vec3 pos2, glm::vec3 v2) {
    glm::vec3 as1,as2,n,d;
    as1 = pos1;
    as2 = pos2;
    n = glm::cross(v1,v2);
    return glm::length(((glm::dot(n, (as1 - as2))) / glm::length(n)));
}

//returns the closest points to each other on two skew lines
std::vector<glm::vec3> getClosestPoints(glm::vec3 as1, glm::vec3 v1, glm::vec3 as2, glm::vec3 v2) {
    glm::vec3 n,n1,n2,d,c1,c2;
    std::vector<glm::vec3> results;
    n = glm::cross(v1, v2);
    n1 = glm::cross(n, v1);
    n2 = glm::cross(n, v2);

    results.push_back(as1 + v1 * (glm::dot(n2,(as2-as1))/glm::dot(v1,n2)));
    results.push_back(as2 + v2 * (glm::dot(n1,(as1-as2))/glm::dot(v2,n1)));
    return results;
}

//A particle with a position, velocity, and mass.
class Particle {
public:

    glm::vec3 pos; //position
    glm::vec3 v; //velocity
    float m; //mass

    //constructor
    Particle(float x, float y, float z, float mass) {
        pos = glm::vec3(x, y, z);
        v = glm::vec3(0.0f,0.0f,0.0f);
        m = mass;
    }

    //step forward in time. Should be called every tick
    virtual void step() {
        pos += v;
    }

    //apply an impulse to the body
    void applyImpulse(glm::vec3 impulse) {
        v += impulse;
    }

    void applyImpulse(float x, float y, float z) {
        v.x += (x/m);
        v.y += (y/m);
        v.z += (z/m);
    }
protected:
    Particle() {
        pos = glm::vec3(0.0f,0.0f,0.0f);
        v = glm::vec3(0.0f,0.0f,0.0f);
        m = 0.0f;
    }
};

class RigidBody: public Particle {
public:
    glm::vec3 rot; //the current angular position of the rigidbody
    glm::vec3 rotV; //the current angular velocity of the rigidbody

    RigidBody(float x,float y,float z,float m) : Particle(x,y,z,m) {
        rot = glm::vec3();
        rotV = glm::vec3();
    }

    void step() {
        Particle::step();
        rot += rotV;
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

    //check for collision between two spheres
    bool checkCollision(SphereBody s1, SphereBody s2) {

        //store the distance between the spheres' vectors
        float l = s1.r + s2.r + 1;

        //get the closest points to each other on the spheres' vectors
        std::vector<glm::vec3> closestPoints = getClosestPoints(s1.pos,s1.v,s2.pos,s2.v);

        //make sure the closest points are on the vectors, not behind or past them
        if(glm::length(closestPoints[0]) <= std::max(glm::length(s1.pos+s1.v),glm::length(s1.pos)) && glm::length(closestPoints[0]) >= std::min(glm::length(s1.pos),glm::length(s1.pos+s1.v))) {
            if(glm::length(closestPoints[1]) <= std::max(glm::length(s2.pos+s2.v),glm::length(s2.pos)) && glm::length(closestPoints[1]) >= std::min(glm::length(s2.pos+s2.v),glm::length(s2.pos))) {
                //record the distance between the vectors
                l = distLineLine(s1.pos, s1.v, s2.pos, s2.v);
            } else {

            }
        } else {
        }

        //check whether the spheres' current location, next tick location, or paths intercept
        if (s1.r + s2.r > glm::length(s2.pos - s1.pos) || s1.r + s2.r > glm::length((s2.pos + s2.v) - (s1.pos + s1.v)) || l < s1.r + s2.r) {
            return true;
        }
        return false;
    }
};

int main() {
    SphereBody myBody = SphereBody(0.0f, 0.0f, 0.0f, 1.0f,2.0f);
    SphereBody otherBody = SphereBody(5.0f, 0.0f, 0.0f, 1.0f,2.0f);
    otherBody.applyImpulse(0,1,0);
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
        std::cout << "--Sphere 1--";
        std::cout << "\nVx: " << myBody.v.x << "  Vy: " << myBody.v.y << "  Vz: " << myBody.v.z;
        std::cout << "\nPos x: " << myBody.pos.x << " Pos y: " << myBody.pos.y << " Pos z: " << myBody.pos.z;
        std::cout << "\n--Sphere 2--";
        std::cout << "\nVx: " << otherBody.v.x << "  Vy: " << otherBody.v.y << "  Vz: " << otherBody.v.z;
        std::cout << "\nPos x: " << otherBody.pos.x << " Pos y: " << otherBody.pos.y << " Pos z: " << otherBody.pos.z;
        std::cout << "\n--Closest Points on Sphere 1's line--";
        std::cout << "\n" << distLineLine(myBody.pos,myBody.v,otherBody.pos,otherBody.v);
        std::cout << "\nX: " << (getClosestPoints(myBody.pos,myBody.v,otherBody.pos,otherBody.v))[0].x;
        std::cout << "  Y: " << (getClosestPoints(myBody.pos,myBody.v,otherBody.pos,otherBody.v))[0].y;
        std::cout << "  Z: " << (getClosestPoints(myBody.pos,myBody.v,otherBody.pos,otherBody.v))[0].z;
        if(myWorld.checkCollision(myBody,otherBody)) {
            std::cout << "\nIntersecting";
        } else {
            std::cout << "\nNot intersecting";
        }
        std::cout << "\n";
    } while(input != 'e');
    return 0;
}
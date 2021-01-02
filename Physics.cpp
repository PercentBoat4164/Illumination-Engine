#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

#include "sources/tiny_obj_loader.h"

#if WIN32
    const std::string MODEL_PATH = "../models/viking_room.obj";
#else
    const std::string MODEL_PATH = "models/viking_room.obj";
#endif

//A body with a position, velocity, and mass.
class Body {
public:

    glm::vec4 pos; //position
    glm::vec4 v; //velocity
    float m; //mass

    //constructor
    Body(float x,float y,float z, float mass) {
        v = glm::vec4(0.0f,0.0f,0.0f, 0.0f);
        pos = glm::vec4(x, y, z, 1.0f);
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
};

class RigidBody: public Body {
public:
    glm::vec3 rot; //the current angular position of the rigidbody
    glm::vec3 rotV; //the current angular velocity of the rigidbody
    float I; //the moment of inertia for the rigidbody
};

//creates a world
class World {
public:

    std::vector<Body*> bodies;

    void addBody(Body *body) {
        bodies.push_back(body);
    }

    void step() {
        for(auto & body : bodies) {
            body -> step();
        }
    }
};

void loadModel() {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
        throw std::runtime_error(warn + err);
    }
}

int main() {



    Body myBody = Body(0.0f,0.0f,0.0f,0.1f);
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
    } while(input != 'e');
    return 0;
}
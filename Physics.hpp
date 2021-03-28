#pragma once

#include "sources/glm/glm.hpp"
#include <iostream>
#include <vector>

//returns the distance between two lines in 3d space
float distLineLine(glm::vec3 pos1, glm::vec3 v1, glm::vec3 pos2, glm::vec3 v2) {
   glm::vec3 as1,as2,n;
   as1 = pos1;
   as2 = pos2;
   n = glm::cross(v1,v2);
   return glm::length(((glm::dot(n, (as1 - as2))) / glm::length(n)));
}

//returns the closest points to each other on two skew lines
std::vector<glm::vec3> getClosestPoints(glm::vec3 as1, glm::vec3 v1, glm::vec3 as2, glm::vec3 v2) {
   glm::vec3 n,n1,n2;
   std::vector<glm::vec3> results;
   n = glm::cross(v1, v2);
   n1 = glm::cross(n, v1);
   n2 = glm::cross(n, v2);

   results.push_back(as1 + v1 * (glm::dot(n2,(as2-as1))/glm::dot(v1,n2)));
   results.push_back(as2 + v2 * (glm::dot(n1,(as1-as2))/glm::dot(v2,n1)));
   return results;
}

class Body {

public:
    glm::vec3 pos;
    glm::vec3 v;

    Body(glm::vec3 position, glm::vec3 velocity) {
        pos = position;
        v = velocity;
    }

    Body(float x, float y, float z) {
        pos = glm::vec3(x,y,z);
        v = glm::vec3(0,0,0);
    }

    Body() {}

    void step() {
       pos += v;
    }
};

class RigidBody: public Body{
public:
    float m;

    RigidBody(float x,float y,float z,float mass) : Body(x,y,z) {
        m = mass;
    }

    RigidBody() {}

    void applyImpulse(glm::vec3 impulse) {
        v += impulse / m;
    }

    void applyImpulse(float x, float y, float z) {
       v.x += (x / m);
       v.y += (y / m);
       v.z += (z / m);
   }
};

class SphereBody: public RigidBody{
public:
    float r;

    SphereBody(float x, float y, float z, float mass, float radius) {
       pos = glm::vec4(x, y, z, 0.0f);
       v = glm::vec3(0,0,0);
       m = mass;
       r = radius;
   }
};

class World {
public:

    std::vector<SphereBody *> activeBodies;
    std::vector<SphereBody *> bodies;

    void addBody(SphereBody *body) {
       bodies.push_back(body);
    }

    void addActiveBody(SphereBody *body) {
        bodies.push_back(body);
        activeBodies.push_back(body);
    }

    void removeBody(SphereBody *body) {
    }

    void activateBody(SphereBody *body) {
        activeBodies.push_back(body);
    }

    void deactivateBody(SphereBody *body) {
    }

    void step() {

        float timeStep = 1;
        
        for(int i = 0; i < activeBodies.size(); i++) {
            for(int j = i+1; j < activeBodies.size(); j++) {
                checkCollision(activeBodies[i], activeBodies[j]);
            }
        }
    }

private:

    //check for collision between two spheres
   bool checkCollision(SphereBody *s1, SphereBody *s2) {

        SphereBody firstBody = *s1;
        SphereBody secondBody = *s2;
        

        //store the distance between the spheres' vectors
        float l = firstBody.r + secondBody.r + 1;

        //get the closest points to each other on the spheres' vectors
        std::vector<glm::vec3> closestPoints = getClosestPoints(firstBody.pos,firstBody.v,secondBody.pos,secondBody.v);

        //make sure the closest points are on the vectors, not behind or past them
        if(glm::length(closestPoints[0]) <= std::max(glm::length(firstBody.pos+firstBody.v),glm::length(firstBody.pos)) && glm::length(closestPoints[0]) >= std::min(glm::length(firstBody.pos),glm::length(firstBody.pos+secondBody.v))) {
            if(glm::length(closestPoints[1]) <= std::max(glm::length(secondBody.pos+secondBody.v),glm::length(secondBody.pos)) && glm::length(closestPoints[1]) >= std::min(glm::length(secondBody.pos+secondBody.v),glm::length(secondBody.pos))) {
                //record the distance between the vectors
                l = distLineLine(firstBody.pos, firstBody.v, secondBody.pos, secondBody.v);
            } else {

            }
        } else {
        }

        //check whether the spheres' current location, next tick location, or paths intercept
        if (secondBody.r + secondBody.r > glm::length(secondBody.pos - firstBody.pos) || firstBody.r + secondBody.r > glm::length((secondBody.pos + secondBody.v) - (firstBody.pos + firstBody.v)) || l < firstBody.r + secondBody.r) {
            return true;
        }
        return false;
   }

    //solves ||(p1 + t*v1) - (p2 + t*v2)|| = r1 + r2 to get the instant of collision and returns the smallest positive value
    float getCollisionTime(SphereBody s1, SphereBody s2) {

        //break down the vectors so we can solve the equation
        float x1 = s1.pos.x;
        float x2 = s2.pos.x;
        float y1 = s1.pos.y;
        float y2 = s2.pos.y;
        float z1 = s1.pos.z;
        float z2 = s2.pos.z;
        float vx1 = s1.v.x;
        float vx2 = s2.v.x;
        float vy1 = s1.v.y;
        float vy2 = s2.v.y;
        float vz1 = s1.v.z;
        float vz2 = s2.v.z;
        float d = s1.r+s2.r;
        
        //use substitution so we can convert the equation into 0=at^2+bt+c and use the quadratic formula
        float cx = x1-x2;
        float cy = y1-y2;
        float cz = z1-z2;

        float cox = vx1-vx2;
        float coy = vy1-vy2;
        float coz = vz1-vz2;

        float a = cox*cox + coy*coy + coz*coz;
        float b = 2 * ((cx * cox) + (cy * coy) + (cz * coz));
        float c = cx*cx + cy*cy + cz*cz - d*d;

        std::cout << "\na,b,c \n";
        std::cout << a << ", " << b << ", " << c;

        //quadratic formula
        float t1 = (-b + std::sqrtf(b*b - 4 * a * c)) / (2 * a);
        float t2 = (-b - std::sqrtf(b*b - 4 * a * c)) / (2 * a);

        std::cout << "\nt1&t2 \n";
        std::cout << t1 << ", " << t2;

        //return the smallest positive solution
        if(t1 > 0) {
            if(t2 > 0) {
                if(t1 < t2) {
                    return t1;
                } else {
                    return t2;
                }
            } else {
                return t1;
            }
        } else if(t2 > 0) {
            return t2;
        }

        //if neither value is positive, return -1
        return -1;
    }

    //manage the elastic collision between two spheres
    void elasticCollide(SphereBody s1, SphereBody s2) {

        //get the line on which the collision takes place. The method calculates a 1d collision along this line
        glm::vec3 collisionNormal = s1.pos - s2.pos;
        collisionNormal *= 1 / glm::length(collisionNormal);

        //the velocity of the spheres on the axis of collision
        glm::vec3 vs1 = collisionNormal * glm::dot(s1.v, collisionNormal);
        glm::vec3 vs2 = collisionNormal * glm::dot(s2.v, collisionNormal);

        //the component of the velocity of the spheres unchanged by the collision
        glm::vec3 remainderS1 = s1.v - vs1;
        glm::vec3 remainderS2 = s2.v - vs2;

        //calculate the collision
        glm::vec3 vS1Result = (s1.m-s2.m)/(s1.m+s2.m)*vs1 + (2*s2.m)/(s1.m+s2.m)*vs2;
        glm::vec3 vS2Result = (s2.m-s1.m)/(s2.m+s1.m)*vs2 + (2*s1.m)/(s2.m+s1.m)*vs1;

        //Recombine the velocity
        s1.v = vS1Result + remainderS1;
        s1.v = vS2Result + remainderS2;
    }
};

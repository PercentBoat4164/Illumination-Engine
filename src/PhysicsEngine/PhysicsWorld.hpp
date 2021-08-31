#pragma once

#include "PhysicsBody.hpp"
#include <vector>
#include "PhysicsCollision.hpp"

//A group of objects that can interact with each other
class PhysicsWorld {
public:
    std::vector<PhysicsBody> bodies; //an array storing all bodies in the world

    //step in time
    void step(float stepTime) {
        float currTime = 0;
        while(currTime < stepTime) {
            
            //find the time of the earliest collision. If none is found, use stepTime.
            float collisionTime = stepTime;
            //@todo actually check for collisions

            //move to all bodies to where they will be at the time of the first collision
            for(int i = 0; i < bodies.size(); i++) {

                //step forward from currTime to collisionTime
                bodies[i].step(collisionTime - (stepTime - currTime)); 
            }
            
            //respond to first collision
        }
    }

    //find the collision time and location between objects. If negative, there is no collision.
    PhysicsCollision checkCollision(PhysicsBody body1, PhysicsBody body2) {
        
        PhysicsCollision collision; //the collision to return
        collision.body1 = body1;
        collision.body2 = body2;

        std::vector<glm::vec3> testTri; //the tri currently being tested
        float testIntersectTime; //the intersection time of the current vertex and tri
        glm::vec3 planeNormal; //the normal of the current tri's plane

        // check each vertex on body1 against each tri on body2, then swap body1 and body2 and do it again
        for(int done = 0; done < 1; done++) {
            
            //swap body1 and body2 after the first iteration
            if(done = 1) {
                PhysicsBody swap = body1;
                body1 = body2;
                body2 = swap;
            }

            //iterate through body1's vertices and body2's tris
            for(int body1Vertex = 0; body1Vertex < body1.mesh.indices.size(); body1Vertex++) {
                for(int body2Index = 0; body2Index < body2.mesh.indices.size(); body2Index += 3) {
                    
                    //get the normal vector of the current tri's plane
                    planeNormal = glm::normalize(glm::cross(body2.mesh.vertices[body2Index], body2.mesh.vertices[body2Index + 1]));
                    
                    //get the time of collision
                    testIntersectTime = getVertexPlaneIntersectTime(body1.mesh.vertices[body1Vertex], body1.velocity, (body2.mesh.getTri(body2Index))[0], planeNormal);
                    
                    //if the collision happens and is the soonest one yet, make it the new first collision
                    if(collision.time < testIntersectTime && collision.time > 0) {
                        testTri = body2.mesh.getTri(body2Index);
                        if(isPointInTriangle(body1.mesh.vertices[body1Vertex] + (body1.velocity * testIntersectTime), testTri)) {
                            collision.time = testIntersectTime;
                            collision.body1Vertex = body1Vertex;
                            collision.body2TriIndex = body2Index;
                        }
                    } else {

                    }
                }
            }
        }
        return collision;
    }

private:

    //get the time of intersection between a plane and a vertex
    float getVertexPlaneIntersectTime(glm::vec3 vertex, glm::vec3 velocity, glm::vec3 planePoint, glm::vec3 planeNormal) {
        
        //make sure lines aren't parallel
        if(glm::dot(vertex, planeNormal) < 0.001) {
            return -1;
        }

        //do the calculation
        return glm::length(velocity) * glm::dot(planePoint - vertex, planeNormal)/(glm::dot(glm::normalize(velocity), planeNormal));
    }

    //check whether a point is in a triangle
    bool isPointInTriangle(glm::vec3 point, std::vector<glm::vec3> tri) {
        std::vector<glm::vec3> normals;
        std::vector<glm::vec3> segments;
        for(int i = 0; i < 2; i++) {
            segments[i] = tri[i] - point;
        }

        normals[0] = glm::normalize(glm::cross(segments[0], segments[1]));
        normals[1] = glm::normalize(glm::cross(segments[1], segments[2]));
        normals[2] = glm::normalize(glm::cross(segments[2], segments[0]));

        if(normals[0] == normals[1] && normals[1] == normals[2]) {
            return true;
        }
        return false;
    }
};
#pragma once

#include <glm/glm.hpp>

//a set of relevant geometry functions
namespace geometryFunctions {

    //returns the distance between points p1 and p2
    float distPointPoint(glm::vec3 p1, glm::vec3 p2);

    //returns the shortest distance between line segment ab and point p1
    float distLinePoint(glm::vec3 a, glm::vec3 b, glm::vec3 p1);

    //returns the shortest distance between line segments a1b1 and a2b2
    float distLineLine(glm::vec3 a1, glm::vec3 b1, glm::vec3 a2, glm::vec3 b2);
}

namespace geometryConstants {

    //used when checking for equals between floats
    const float minError = 0.001;
}

//A three-dimensional shape that can check for collisions with other shapes
class Shape {
    public:

        glm::vec3 position; //the position of the shape
        glm::vec3 rotation; //the rotation of the shape

        //returns whether a point represented by a position vector is inside the shape
        virtual bool isInside(glm::vec3 target);

        //returns whether a line is intersecting any part of the shape
        virtual bool isIntersecting(Shape otherShape);

        //returns the closest point inside the shape to the target
        virtual glm::vec3 getClosestPoint(glm::vec3 target);
};

//A point. All the default shape methods work for points.
class Point : Shape {
    Point(glm::vec3 pos);
};

//It's a sphere
class Sphere : Shape {
    public:
        float radius; //the radius of the sphere

        Sphere(glm::vec3 pos, float r);

        bool isInside(glm::vec3 point);

        bool isIntersecting(Shape otherShape);

        glm::vec3 getClosestPoint(glm::vec3 target);
};

//a plane defined by three vertices
class Plane : Shape {
    public:
        glm::vec3 vert1, vert2, vert3; //the vertices of the tri that defines the plane

        Plane(glm::vec3 vertex1, glm::vec3 vertex2, glm::vec3 vertex3);

        Plane(glm::vec3 location, glm::vec3 normal);

        bool isInside(glm::vec3 point);

        glm::vec3 getClosestPoint(glm::vec3 target);
};

//a line segment represented by a position and a vector
class LineSegment : Shape {
    public:
        glm::vec3 segment; //a vector representing the line segment, starting from the linesegment's position

        LineSegment(glm::vec3 pos, glm::vec3 segmentVector);

        bool isInside(glm::vec3 target);

        bool isIntersecting(Shape otherShape);

        glm::vec3 getClosestPoint(glm::vec3 target);

    protected:

        glm::vec3 getLengthAlongLine(glm::vec3 target);

        bool isBeyondSegment(glm::vec3 target);

        bool isBeforeSegment(glm::vec3 target);
};


//incomplete
/*
class Cylinder : LineSegment {
    public:
        float radius; //radius of the circle around the cylinder's line segment
};

//a triangle in 3d space. A child of Plane
class Tri : Plane {
    public:
};*/
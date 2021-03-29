#pragma once

#include "../../glm/glm.hpp"

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
};

//It's a sphere
class Sphere : Shape {
    public:
        float radius; //the radius of the sphere

        bool isInside(glm::vec3 point);

        bool isIntersecting(Shape otherShape);

        glm::vec3 getClosestPoint(glm::vec3 target);
};

class LineSegment : Shape {
    public:
        glm::vec3 segment; //a vector representing the line segment, starting from the linesegment's position
};

class Tri : Shape {
    public:
        glm::vec3 vert1, vert2, vert3; //the vertices of the tri


};

class Plane : Shape {
    public:
        glm::vec3 vert1, vert2, vert3; //the vertices of the tri that defines the plane
};
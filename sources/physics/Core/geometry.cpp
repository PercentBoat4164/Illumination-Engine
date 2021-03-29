#include "geometry.hpp"

//A three-dimensional shape that can check for collisions with other shapes
class Shape {
    public:

        glm::vec3 position; //the position of the shape
        glm::vec3 rotation; //the rotation of the shape

        //returns whether a point represented by a position vector is inside the shape
        virtual bool isInside(glm::vec3 target) {
            if(target == position) {
                return true;
            }
            return false;
        }

        //returns whether a line is intersecting any part of the shape
        virtual bool isIntersecting(Shape otherShape) {
            if(otherShape.getClosestPoint(position) == position) {
                return true;
            }
            return false;
        }

        //returns the closest point inside the shape to the target
        virtual glm::vec3 getClosestPoint(glm::vec3 target) {
            return position;
        };
};

//Its a sphere
class Sphere : Shape {
    public:
        float radius; //the radius of the sphere

        bool isInside(glm::vec3 point) {
            if(glm::length(point - position) < radius) {
                return true;
            }
            return false;
        }

        bool isIntersecting(Shape otherShape) {
            if(glm::length(otherShape.getClosestPoint(position) - position) < radius) {
                return true;
            }
            return false;
        }

        glm::vec3 getClosestPoint(glm::vec3 target) {
            glm::vec3 result = target;
            glm::vec3 calculatedLine = target - position;
            if(glm::length(calculatedLine) > radius) {
                result = position + (glm::normalize(calculatedLine) * radius);
            }
            return result;
        }
};

class LineSegment : Shape {
    public:
        glm::vec3 segment; //a vector representing the line segment, starting from the linesegment's position

        bool isInside(glm::vec3 target) {
            
        }

        bool isIntersecting(Shape otherShape);

        glm::vec3 getClosestPoint(glm::vec3 target);
};

class Tri : Shape {
    public:
        glm::vec3 vert1, vert2, vert3; //the vertices of the tri


};

class Plane : Shape {
    public:
        glm::vec3 vert1, vert2, vert3; //the vertices of the tri that defines the plane
};
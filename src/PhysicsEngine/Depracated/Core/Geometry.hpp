#pragma once

#include <glm/glm.hpp>

//a set of relevant geometry functions
namespace GeometryFunctions {

    //returns the distance between points p1 and p2
    float distPointPoint(glm::vec3 p1, glm::vec3 p2) {
        return glm::length(p2-p1);
    };

    //returns the shortest distance between line ab and point p1
    float distLinePoint(glm::vec3 a, glm::vec3 b, glm::vec3 p1) {
        return glm::length(glm::cross(p1-b, b-a) / glm::length(b-a));
    }

    //returns the shortest distance between line segments a1b1 and a2b2
    float distLineLine(glm::vec3 a1, glm::vec3 b1, glm::vec3 a2, glm::vec3 b2) {
        glm::vec3 n, ab1, ab2;
        ab1 = b1-a1;
        ab2 = b2-a2;
        n = glm::cross(ab1, ab2);
        return glm::length((glm::dot(n, (a1 - a2))) / glm::length(n));
    }
}

namespace GeometryConstants {

    //used when checking for equals between floats
    const float minError = 0.001f;
}


//A three-dimensional shape that can check for collisions with other shapes
class Shape {
    public:

        glm::vec3 position; //the position of the shape
        glm::vec3 rotation; //the rotation of the shape
        float radius; //the radius of the sphere

        //returns whether a point represented by a position vector is inside the shape
        virtual bool isInside(glm::vec3 target) {
            if(target == position) {
                return true;
            }
            return false;
        }

        //returns whether a line is intersecting any part of the shape
        virtual bool isIntersecting(Shape otherShape) {
            return isInside(otherShape.getClosestPoint(position));
        }

        //returns the closest point inside the shape to the target
        virtual glm::vec3 getClosestPoint(glm::vec3 target) {
            return position;
        };
};

//A point. All the default shape methods work for points.
class Point : public Shape {
    Point(glm::vec3 pos) {
        position = pos;
    }
};

class Sphere : public Shape {
    public:

        Sphere(glm::vec3 pos, float r) {
            position = pos;
            radius = r;
        }

        bool isInside(glm::vec3 target) {
            //create a vector from the center of the sphere to the target.
            glm::vec3 result = target;
            glm::vec3 calculatedLine = target - position;

            //if the vector is longer than r return false
            if(glm::length(calculatedLine) <= radius) {
                return true;
            }
            //otherwise return true
            return false;
        }

        glm::vec3 getClosestPoint(glm::vec3 target) {

            //create a vector from the center of the sphere to the target.
            glm::vec3 calculatedLine = target - position;

            //if the target is outside the sphere, return the point on the edge of the sphere
            //and on the vector between the sphere's center and the target
            if(!isInside(target)) {
                return position + (glm::normalize(calculatedLine) * radius);
            }

            //otherwise return the target
            return target;
        }

        bool isIntersecting(Shape otherShape) {
            return isInside(otherShape.getClosestPoint(position));
        }

        Sphere getBoundingSphere() {
            return Sphere(position, radius);
        }
};

//planes
class Plane : Shape {
    public:
        glm::vec3 vert1, vert2, vert3; //the vertices of the tri that defines the plane

        //define a plane with three vertices
        Plane(glm::vec3 vertex1, glm::vec3 vertex2, glm::vec3 vertex3) {
            vert1 = vertex1;
            vert2 = vertex2;
            vert3 = vertex3;
        }

        //define a plane from a point and a vector normal to it
        Plane(glm::vec3 location, glm::vec3 normal) {

            //set the first vertex to be at the location point
            vert1 = location;

            //set the other vertices to the normal vector rotated 90 degrees in different directions
            vert2 = location + glm::vec3(-1 * normal.y, normal.x, normal.z);
            vert3 = location + glm::vec3(normal.x, -1 * normal.z, normal.y);
        }

        bool isInside(glm::vec3 point) {
            //create a matrix of points to see if they share a plane. If they are not, the target is not inside the triangle.
            glm::mat4 potentialPlane = glm::mat4(
                glm::vec4(vert1.x, vert1.y, vert1.z, 1),
                glm::vec4(vert2.x, vert2.y, vert2.z, 1), 
                glm::vec4(vert3.x, vert3.y, vert3.z, 1), 
                glm::vec4(point.x, point.y, point.z, 1));

            //make sure the point is on the plane
            return glm::determinant(potentialPlane) < GeometryConstants::minError;
        }

        glm::vec3 getClosestPoint(glm::vec3 target) {

            //create a vector normal to the plane
            glm::vec3 planeNormal = glm::normalize(glm::cross(vert2 - vert1, vert3 - vert1));
            glm::vec3 linetoVector = target - vert1;
            float distance = glm::dot(linetoVector, planeNormal);
            return target - (linetoVector * distance);
        }  
};

class LineSegment : Shape {
    public:
        glm::vec3 segment; //a vector representing the line segment, starting from the linesegment's position

        LineSegment(glm::vec3 pos, glm::vec3 segmentVector) {
            position = pos;
            segment = segmentVector;
        }

        /*LineSegment(glm::vec3 p1, glm::vec3 p2) {
            position = p1;
            segment = p2 - p1;
        }*/

        bool isInside(glm::vec3 target) {
            return GeometryFunctions::distLinePoint(position, position + segment, target) == 0;
        }

        bool isIntersecting(Shape otherShape) {
            return GeometryFunctions::distLinePoint(position, position + segment, otherShape.getClosestPoint(position)) == 0;
        }

        glm::vec3 getClosestPoint(glm::vec3 target) {
            glm::vec3 pointOnLine = getLengthAlongLine(target);

            //if the closest point is behind the segment, return the first point in the segment
            if(isBeforeSegment(target)) {
                return position;

            //if the point is beyond the segment, return the position of the other end of the segment
            } else if(isBeyondSegment(target)) {
                return position + segment;
            }
            //otherwise return the closest point, which must be on the segment
            return pointOnLine;
        }

    protected:

        //returns the vector from the line segment's position to the closest point to the target on its line
        glm::vec3 getLengthAlongLine(glm::vec3 target) {
            glm::vec3 ac = position - target;
            glm::vec3 adNormal = glm::normalize(segment);
            return adNormal * glm::dot(ac, adNormal);
        }
        
        //returns whether the closest point to the target on the line is farther than the last point on the segment
        bool isBeyondSegment(glm::vec3 target) {
            return glm::length(getLengthAlongLine(target)) > glm::length(segment);
        }

        //returns whether the closest point to the target on the line is before the first point on the segment
        bool isBeforeSegment(glm::vec3 target) {
            return glm::normalize(segment) == glm::normalize(target - position) * -1.0f;
        }
};


//not yet implemented
/*
class Cylinder : LineSegment {
    public:

        float radius; //radius of the circle

        Cylinder(glm::vec3 pos, glm::vec3 segmentVector, float r) {
            position = pos;
            segment = segmentVector;
            radius = r;
        }

        bool isInside(glm::vec3 target) {
            if(!isBeforeSegment(target) && !isBeyondSegment(target) && geometryFunctions.distLinePoint(target) < r) {
                return true;
            }
            return false;
        }

        glm::vec3 getClosestPoint(glm::vec3 target) {
            if(isBeforeSegment(target)) {
                glm::vec3 potentialPoint = Plane(position, segment).getClosestPoint()
                if(isInside(potentialPoint)) {
                    return potentialPoint
                } else {
                    
                }
            } else if(isBeyondSegment(target)) {
                return geometryFunctions::distPointPoint(target, Plane(position + segment, segment));
            } else if(geometryFunctions::distLinePoint(position, position + segment, target) < r) {
                p1 = position + getLengthAlongLine(target);
                return r * glm::normalize(target - p1) + p1;
            }
            return target;
        }
};

class Tri : Plane {
    public:
        bool isInside(glm::vec3 target) {
            if(Plane::isInside(target)) {
                
            }
        }
};
*/
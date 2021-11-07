#pragma once

#include "../Core/Path.hpp"
#define PI 3.141592f

//a class that moves objects in a circle
class CircularPath: public Path {
public:
    glm::vec3 center{glm::vec3(0,0,0)}; //the center of the circle
    float radius{15.0f}; //the radius of the circle
    float radiansPerSecond{PI / 2}; //the speed of rotation
    float currTime{0.0f}; //the current time

    void step(float time) {
        currTime = time;
        float angle = currTime * radiansPerSecond;
        float X = glm::cos(angle);
        float Y = glm::sin(angle);
        *position = center + (radius * glm::vec3(X, Y, 0));
        //*rotation = glm::quat(1, 0, 0, 0);
    }
};
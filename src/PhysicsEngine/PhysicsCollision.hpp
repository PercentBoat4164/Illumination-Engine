#pragma once

#include <glm.hpp>
#include "PhysicsBody.hpp"

struct PhysicsCollision {
    float time;
    PhysicsBody body1;
    PhysicsBody body2;
    int body1Vertex;
    int body2TriIndex;
};
#pragma once

#include "../Core/Body.hpp"
#include "../Core/Geometry.hpp"


class RigidBody : public Body {
public:

    RigidBody(Shape *newShape) {
        shape = newShape;
    }
};
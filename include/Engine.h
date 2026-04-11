#ifndef ENGINE_H
#define ENGINE_H
#include <glm/gtx/string_cast.hpp>

#include "phi.h"

struct Engine {
    float throttle = 1.0f, thrust;

    Engine(float thrust) : thrust(thrust) {}

    void apply_force(phi::RigidBody *rigid_body)
    {
        rigid_body->add_relative_force(phi::FORWARD * throttle * thrust);
    }
};

#endif
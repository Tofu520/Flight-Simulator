#ifndef ENGINE_H
#define ENGINE_H
#include <glm/gtx/string_cast.hpp>

#include "phi.h"
#include "isa.h"

struct Engine {
    float throttle = 0.5f, thrust;

    Engine(float thrust) : thrust(thrust) {}

    void apply_force(phi::RigidBody *rigid_body, float altitude)
    {
        float h = glm::clamp(altitude, 0.0f, 11000.0f);
        //engine thrust decreases in higher altitudes
        float sigma = isa::get_air_density(h) / isa::sea_level_air_density;
        rigid_body->add_relative_force(phi::FORWARD * throttle * thrust * sigma);
    }
};

#endif
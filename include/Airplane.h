#ifndef AIRPLANE_H
#define AIRPLANE_H

#include "Airfoil.h"
#include "Wing.h"
#include "Engine.h"
#include "isa.h"

struct Airplane : public phi::RigidBody {
    Engine engine;
    std::vector<Wing> elements;
    float pitch_control = 0.0f;  
    float roll_control = 0.0f;   
    
    Airplane() : engine(50000.0f) {
        mass = 10000.0f;
        glm::mat3 I = glm::mat3(
            48531.0f, -1320.0f,     0.0f,
            -1320.0f,  256608.0f,    0.0f,
            0.0f,     0.0f,      211333.0f
        );
        set_inertia(I);
    }

    void update(float dt) override
    {
        engine.apply_force(this);

        if (elements.size() >= 6) {
            elements[0].set_control_input(0.0f);          //left main wing 
            elements[1].set_control_input(0.0f);          //right main wing 
            elements[2].set_control_input(-roll_control); // left aileron
            elements[3].set_control_input(roll_control);  // right aileron
            elements[4].set_control_input(-pitch_control); // elevator
            elements[5].set_control_input(0.0f);          //rudder
        }

        for (auto& wing : elements){
            wing.apply_forces(this, dt);
        }
            

        phi::RigidBody::update(dt);
    }
};

#endif
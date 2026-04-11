#ifndef WING_H
#define WING_H

#include "Airfoil.h"
#include <glm/glm.hpp>
#include "phi.h"
#include "isa.h"

struct Wing {
  float area;
  float wingspan;
  float chord;
  float aspect_ratio;
  const Airfoil* airfoil;
  glm::vec3 normal;
  glm::vec3 center_of_pressure;
  float flap_ratio;  
  float efficiency_factor = 1.0f;

  float control_input = 0.0f;

  Wing(const Airfoil* airfoil, const glm::vec3& relative_position, float area, float span, const glm::vec3& normal,
       float flap_ratio = 0.25f)
      : airfoil(airfoil),
        center_of_pressure(relative_position),
        area(area),
        chord(area / span),
        wingspan(span),
        normal(normal),
        aspect_ratio(phi::sq(span) / area),
        flap_ratio(flap_ratio)
  {
  }

  Wing(const glm::vec3& position, float span, float chord, const Airfoil* airfoil, const glm::vec3& normal,
       float flap_ratio = 0.25f)
      : airfoil(airfoil),
        center_of_pressure(position),
        area(span * chord),
        chord(chord),
        wingspan(span),
        normal(normal),
        aspect_ratio(phi::sq(span) / area),
        flap_ratio(flap_ratio)
  {
  }

    void set_control_input(float input) { control_input = glm::clamp(input, -1.0f, 1.0f); }

    void apply_forces(phi::RigidBody* rigid_body, phi::Seconds dt)
    {

      glm::vec3 local_velocity = rigid_body->get_point_velocity(center_of_pressure);
      float speed = glm::length(local_velocity);

      if (speed < 1.0f) return;

      glm::vec3 drag_direction = glm::normalize(-local_velocity);

      glm::vec3 lift_direction = glm::normalize(glm::cross(glm::cross(drag_direction, normal), drag_direction));

      float angle_of_attack = glm::degrees(std::asin(glm::dot(drag_direction, normal)));

      auto [lift_coeff, drag_coeff] = airfoil->sample(angle_of_attack);

      if (flap_ratio > 0.0f) {
        float delta_lift_coeff = sqrt(flap_ratio) * airfoil->cl_max * control_input;
        lift_coeff += delta_lift_coeff;
      }

      float induced_drag_coeff = phi::sq(lift_coeff) / (phi::PI * aspect_ratio * efficiency_factor);
      drag_coeff += induced_drag_coeff;

      float air_density = isa::get_air_density(rigid_body->position.y);

      float dynamic_pressure = 0.5f * phi::sq(speed) * air_density * area;

      glm::vec3 lift = lift_direction * lift_coeff * dynamic_pressure;
      glm::vec3 drag = drag_direction * drag_coeff * dynamic_pressure;
      rigid_body->add_force_at_point(lift + drag, center_of_pressure);
    }

    static glm::vec3 calc_wing_normal(const glm::vec3& normal, float incidence)
    {
      auto axis = glm::normalize(glm::cross(phi::FORWARD, normal));
      auto rotation = glm::rotate(glm::mat4(1.0f), glm::radians(incidence), axis);
      return glm::vec3(rotation * glm::vec4(normal, 1.0f));
    }
    
};


#endif
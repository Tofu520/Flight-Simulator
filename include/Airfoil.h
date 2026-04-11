#ifndef AIRFOIL_H
#define AIRFOIL_H

#include <vector>
#include <tuple>
#include <glm/glm.hpp>
#include "phi.h"

struct Airfoil {
    float min_alpha, max_alpha;
    float cl_max;
    int max_index;
    std::vector<glm::vec3> data;

    Airfoil(const std::vector<glm::vec3> &curve) : data(curve)
    {
        max_index = static_cast<int>(data.size() - 1);
        min_alpha = curve[0].x;
        max_alpha = curve[curve.size() - 1].x;

        //compute cl_max from the data instead of leaving it at 0
        cl_max = 0.0f;
        for (const auto& point : data) {
            cl_max = std::max(cl_max, point.y);
        }
        
    }

    // lift_coeff, drag_coeff
    std::tuple<float, float> sample(float alpha) const
    {
        float t = phi::inverse_lerp(min_alpha, max_alpha, alpha) * max_index;
        float integer = std::floor(t);
        float fractional = t - integer;
        int index = static_cast<int>(integer);
        auto value = (index < max_index) ? phi::lerp(data[index], data[index + 1], fractional) : data[max_index];
        return {value.y, value.z};
    }

};

#endif
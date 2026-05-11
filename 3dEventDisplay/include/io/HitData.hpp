#pragma once

#include <glm/glm.hpp>

namespace snd3D {
    struct HitData {
        glm::vec3 position;
        glm::vec3 radius;
        double energy;
        double time;

        HitData(double x, double y, double z, double radiusX, double radiusY, double radiusZ, double _energy, double _time)
            : position((float)x, (float)y, (float)z), radius((float)radiusX, (float)radiusY, (float)radiusZ), energy(_energy), time(_time) {}
    };
}

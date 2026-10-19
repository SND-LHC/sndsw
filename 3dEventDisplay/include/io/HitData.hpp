/**
 * @file HitData.hpp
 * @brief Defines the structure for individual detector hit data.
 *
 * HitData represents a single recorded hit from a detector channel, containing
 * 3D position coordinates, dimensions (radius in each axis), energy deposited,
 * and timing information (clock cycle). Provides a lightweight struct for passing
 * hit information through the visualization pipeline.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-24
 */

#pragma once

#include <glm/glm.hpp>

namespace snd3D {
    struct HitData {
        glm::vec3 position;
        glm::vec3 radius;
        double energy;
        double clockCycle;

        HitData(double x, double y, double z, double radiusX, double radiusY, double radiusZ, double _energy, double _clockCycle)
            : position((float)x, (float)y, (float)z), radius((float)radiusX, (float)radiusY, (float)radiusZ), energy(_energy), clockCycle(_clockCycle) {}
    };
}

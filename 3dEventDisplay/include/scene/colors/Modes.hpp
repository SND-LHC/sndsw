/**
 * @file Modes.hpp
 * @brief Enumerations for color scaling and variable selection modes.
 *
 * Defines enumerations for the two main strategy selection axes in the color system:
 * ColorScalingMode (how values map to colors: linear, logarithmic, fixed) and
 * ColorVariable (which hit property to use: energy, time, clock cycle).
 *
 * @author Enrico Bartocetti
 * @date 2026-05-14
 */

#pragma once

namespace snd3D {
    enum class ColorScalingMode {
        LINEAR,
        LOGARITHMIC,
        FIXED
    };

    enum class ColorVariable {
        ENERGY,
        TIME,
        CLOCK_CYCLE
    };
}

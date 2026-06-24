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

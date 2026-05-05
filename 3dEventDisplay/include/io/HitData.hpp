#pragma once

namespace snd3D {
    struct HitData {
        const double x;
        const double y;
        const double z;
        const double radiusX;
        const double radiusY;
        const double radiusZ;

        HitData(double _x, double _y, double _z, double _radiusX, double _radiusY, double _radiusZ)
            : x(_x), y(_y), z(_z), radiusX(_radiusX), radiusY(_radiusY), radiusZ(_radiusZ) {}
    };
}

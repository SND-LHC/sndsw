#pragma once

#include <string>
#include <vector>
#include <memory>

#include "io/HitData.hpp"

namespace snd3D {
    class DetectorData {
        public:
            std::pair<double, double> energyRange;
            std::pair<double, double> timeRange;

            DetectorData(std::string _name);
            void addHit(HitData* hit);
            const std::vector<std::unique_ptr<HitData>>& getHits() const;
            std::string getName();

        private:
            std::vector<std::unique_ptr<HitData>> hits;
            std::string name;
    };
}

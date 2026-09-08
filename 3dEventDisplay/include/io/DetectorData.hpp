/**
 * @file DetectorData.hpp
 * @brief Manages detector-specific hit data and properties.
 *
 * DetectorData represents a single detector's contribution to an event, containing
 * all hits recorded by that detector along with energy and clock cycle range metadata.
 * Provides accessors to retrieve hit collections and detector identity information.
 * Serves as a container for organizing hits by their originating detector.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-24
 */

#pragma once

#include <string>
#include <vector>
#include <memory>

#include "io/HitData.hpp"

namespace snd3D {
    class DetectorData {
        public:
            std::pair<double, double> energyRange;
            std::pair<double, double> clockCycleRange;

            DetectorData(std::string _name);
            void addHit(HitData* hit);
            const std::vector<std::unique_ptr<HitData>>& getHits() const;
            std::string getName();

        private:
            std::vector<std::unique_ptr<HitData>> hits;
            std::string name;
    };
}

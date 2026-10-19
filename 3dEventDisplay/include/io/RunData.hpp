/**
 * @file RunData.hpp
 * @brief Defines the structure for holding run metadata.
 *
 * RunData represents metadata for a single detector run, including run number,
 * start date, geometry name, and total event count. Used as a container to pass
 * run information from the I/O system to the visualization system.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-24
 */

#pragma once

#include <string>

namespace snd3D {
    struct RunData {
        const int runNumber;
        const std::string startDate;
        const std::string geoName;
        const int totalEvents;

        RunData(int id, std::string date, std::string _geoName, int events)
            : runNumber(id), startDate(std::move(date)), geoName(std::move(_geoName)), totalEvents(events) {}
    };
}

#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <memory>

#include "io/HitData.hpp"

namespace snd3D {
    class EventData {
        public:
            const int id;
            const std::string dateTime;
            const int64_t timestamp;

            EventData(int _id, std::string _dateTime, int64_t _timestamp);
            void addHit(HitData* hit);
            const std::vector<std::unique_ptr<HitData>>& getHits() const;


        private:
            std::vector<std::unique_ptr<HitData>> hits;
    };
}

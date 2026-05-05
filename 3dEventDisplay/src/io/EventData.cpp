#include "io/EventData.hpp"

namespace snd3D {
        EventData::EventData(int _id, std::string _dateTime, int64_t _timestamp)
                : id(_id), dateTime(std::move(_dateTime)), timestamp(_timestamp) {
        }

        void EventData::addHit(HitData* hit) {
            this->hits.push_back(std::unique_ptr<HitData>(hit));     
        }

        const std::vector<std::unique_ptr<HitData>>& EventData::getHits() const {
            return this->hits;
        }
}

#include "io/DetectorData.hpp"

namespace snd3D {

    DetectorData::DetectorData(std::string _name) {
        this->name = _name;
    }

    void DetectorData::addHit(HitData* hit) {
        this->hits.push_back(std::unique_ptr<HitData>(hit));     
    }

    const std::vector<std::unique_ptr<HitData>>& DetectorData::getHits() const {
        return this->hits;
    }

    std::string DetectorData::getName() {
        return this->name;
    }

}

#include "io/SndswEventManager.hpp"

#include <utility>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cstdint>
#include <iostream>
#include <stdexcept>

#include "sndTchainGetter.h"
#include "sndScifiPlane.h"
#include "sndPlaneTools.h"

#include "io/HitData.hpp"
#include "io/DetectorData.hpp"
#include "io/EventData.hpp"
#include "io/RunData.hpp"
#include "io/GeometryMismatchException.hpp"

namespace snd3D {

    SndswEventManager::~SndswEventManager() {
        if (this->scifiGeometry != nullptr)     delete this->scifiGeometry;
        if (this->mufilterGeometry != nullptr)  delete this->mufilterGeometry;
        if (this->config != nullptr)            delete this->config;
        if (this->boundaries != nullptr)        delete this->boundaries;
        if (this->muHits != nullptr)            delete this->muHits;
        if (this->sfHits != nullptr)            delete this->sfHits;
        if (this->header != nullptr)            delete this->header;
    }

    RunData* SndswEventManager::loadRun(int64_t runNumber) {

        // LOAD GEOMETRY FILENAME
        std::string fullPath = snd::analysis_tools::GetGeoPath(runNumber); 
        size_t lastSlash = fullPath.find_last_of('/');
        std::string fileName = (lastSlash == std::string::npos) ? fullPath : fullPath.substr(lastSlash + 1);

        size_t lastDot = fileName.find_last_of('.');
        if (lastDot != std::string::npos && fileName.substr(lastDot) == ".root") {
            fileName = fileName.substr(0, lastDot);
        }

        if (!this->loadedGeometry.empty() && fileName != this->loadedGeometry) {
            throw GeometryMismatchException(this->loadedRun, this->loadedGeometry, runNumber, fileName);
        }

        std::unique_ptr<TChain> newChain = snd::analysis_tools::GetTChain(runNumber);
        if (newChain->GetEntries() <= 0) {
            throw std::runtime_error("Run " + std::to_string(runNumber) + " is empty:\nNo events found in the ROOT chain.");
        }

        this->chain = std::move(newChain);

        if (this->muHits != nullptr) delete this->muHits;
        if (this->sfHits != nullptr) delete this->sfHits;
        if (this->header != nullptr) delete this->header;

        this->muHits = new TClonesArray("MuFilterHit");
        this->chain->SetBranchAddress("Digi_MuFilterHits", &this->muHits);
        this->sfHits = new TClonesArray("sndScifiHit");
        this->chain->SetBranchAddress("Digi_ScifiHits", &this->sfHits);
        this->header = new SNDLHCEventHeader();
        this->chain->SetBranchAddress(
                this->chain->GetBranch("EventHeader") ? "EventHeader" : "EventHeader.",
                &this->header
        );

        // LOAD RUN DATE AS THE DATE OF THE FIRST EVENT
        this->chain->GetEntry(0);
        int64_t rawTime = this->header->GetUTCtimestamp();
        time_t t = static_cast<time_t>(rawTime);
        struct tm *dt = std::gmtime(&t);
        std::stringstream ss;
        ss << std::put_time(dt, "%Y-%m-%d");

        this->loadedRun = runNumber;

        return new RunData(runNumber, ss.str(), fileName, this->chain->GetEntries());
    }

    void SndswEventManager::loadGeometry() {
        if (this->loadedRun == -1) {
            throw std::runtime_error("Run not loaded yet: loadRun() needs to be called.");
        }

        std::pair<Scifi*, MuFilter*> geometry = snd::analysis_tools::GetGeometry(this->loadedRun);
        this->scifiGeometry = geometry.first;
        this->mufilterGeometry = geometry.second;
        this->config = new snd::Configuration(snd::Configuration::GetOption(this->loadedRun), this->scifiGeometry, this->mufilterGeometry);
        this->boundaries = new snd::analysis_tools::DetectorBoundaries(*this->config, this->loadedRun); 

        this->scifiGeometry->InitEvent(this->header);
        this->mufilterGeometry->InitEvent(this->header);

        this->scifiPlanes.clear();
        this->usPlanes.clear();

        std::string fullPath = snd::analysis_tools::GetGeoPath(this->loadedRun); 
        size_t lastSlash = fullPath.find_last_of('/');
        std::string fileName = (lastSlash == std::string::npos) ? fullPath : fullPath.substr(lastSlash + 1);

        size_t lastDot = fileName.find_last_of('.');
        if (lastDot != std::string::npos && fileName.substr(lastDot) == ".root") {
            fileName = fileName.substr(0, lastDot);
        }

        this->loadedGeometry = fileName;
    }

    EventData* SndswEventManager::loadEvent(int64_t eventNumber, int minScifiEntries, int minUsEntries) {

        if (this->loadedRun == -1) {
            throw std::runtime_error("Run not loaded yet: loadRun() needs to be called.");
        }

        if (this->scifiGeometry == nullptr || this->mufilterGeometry == nullptr) {
            throw std::runtime_error("Geometry not loaded yet: loadGeometry() needs to be called.");
        }

        if (eventNumber < 0 || eventNumber >= this->chain->GetEntries()) {
            throw std::out_of_range("Invalid Event Number: must be between 0 and " + std::to_string(this->chain->GetEntries() - 1));
        }

        this->chain->GetEntry(eventNumber);

        int64_t rawTime = this->header->GetUTCtimestamp();
        time_t t = static_cast<time_t>(rawTime);
        struct tm *dt = std::gmtime(&t);
        std::stringstream ss;
        ss << std::put_time(dt, "%Y-%m-%d %H:%M:%S");

        EventData* toReturn = new EventData(eventNumber, ss.str(), rawTime);

        this->vetoPlanes = snd::analysis_tools::FillVeto(*this->config, this->muHits, this->mufilterGeometry);
        this->scifiPlanes = snd::analysis_tools::FillScifi(*this->config, this->sfHits, this->scifiGeometry);
        this->usPlanes = snd::analysis_tools::FillUS(*this->config, this->muHits, this->mufilterGeometry);
        this->dsPlanes = snd::analysis_tools::FillDS(*this->config, this->muHits, this->mufilterGeometry);

        if (this->sfHits->GetEntries() < minScifiEntries) {
            return toReturn;
        }
        int count{0};
        for (const auto &p : this->usPlanes) {
          count += (p.GetNHits().large);
        }
        if (count < minUsEntries) {
            return toReturn;
        }

        // cluster SCIFI
        //std::cout << "########################### SCIFI #####################" << std::endl;
        std::vector<std::vector<snd::analysis_tools::Cluster>> scifi_clusters(this->config->scifi_n_stations);
        DetectorData* detector = new DetectorData("SciFi");
        toReturn->addDetector(detector);

        for (auto &p : this->scifiPlanes) {
            int st = p.GetStation() - 1;

            if (st >= 0 && st < this->config->scifi_n_stations+1) {
                //std::cout << p.GetHits().size() << " hits in Scifi plane " << st << std::endl;
                auto plane_clusters = ClustersPositions(*this->boundaries, p.GetHits(), this->config->scifi_centroid_error_x, this->config->scifi_centroid_error_y, 1.5, this->config->scifi_qdc_to_gev, 2);    // centroid error is fiber width which is the same for x and y

                scifi_clusters[st].insert(
                    scifi_clusters[st].end(),
                    plane_clusters.begin(),
                    plane_clusters.end()
                );
            }
        }

        for (int i = 0; i < this->config->scifi_n_stations; ++i) {
            for (auto &c : scifi_clusters[i]) {
                //std::cout << "Scifi Station " << (i) << ": " << c.center << "\t" << c.radius << std::endl;
                detector->addHit(new HitData(c.center.X(), c.center.Y(), c.center.Z(), c.radius.X(), c.radius.Y(), c.radius.Z(), c.energy, c.time));
            }
        }

        detector->energyRange = snd::analysis_tools::FindRange(scifi_clusters);
        detector->timeRange = snd::analysis_tools::FindRange(scifi_clusters, true);

        // cluster VETO
        //std::cout << "########################### VETO #####################" << std::endl;
        std::vector<std::vector<snd::analysis_tools::Cluster>> veto_clusters(this->config->veto_n_stations);
        detector = new DetectorData("Veto");
        toReturn->addDetector(detector);

        for (auto &p : this->vetoPlanes) {
            int st = p.GetStation() - 1;

            if (st >= 0 && st < this->config->veto_n_stations+1) {
                //std::cout << p.GetHits().size() << " hits in Veto plane " << st << std::endl;
                auto plane_clusters = ClustersPositions(*this->boundaries, p.GetHits(),1.73, 3, 7);

                veto_clusters[st].insert(
                    veto_clusters[st].end(),
                    plane_clusters.begin(),
                    plane_clusters.end()
                );
            }
        }

        for (int i = 0; i < this->config->veto_n_stations; ++i) {
            for (auto &c : veto_clusters[i]) {
                //std::cout << "Veto Station " << (i) << ": " << c.center << "\t" << c.radius << std::endl;
                detector->addHit(new HitData(c.center.X(), c.center.Y(), c.center.Z(), c.radius.X(), c.radius.Y(), c.radius.Z(), c.energy, c.time));
            }
        }

        // cluster US
        //std::cout << "########################### US #####################" << std::endl;
        std::vector<std::vector<snd::analysis_tools::Cluster>> us_clusters(this->config->us_n_stations);
        detector = new DetectorData("US");
        toReturn->addDetector(detector);

        for (auto &p : this->usPlanes) {
            int st = p.GetStation() - 1;

            if (st >= 0 && st < this->config->us_n_stations+1) {
                //std::cout << p.GetHits().size() << " hits in US plane " << st << std::endl;
                auto plane_clusters = ClustersPositions(*this->boundaries, p.GetHits(), this->config->us_centroid_error_x, this->config->us_centroid_error_y, 7, this->config->us_qdc_to_gev);

                us_clusters[st].insert(
                    us_clusters[st].end(),
                    plane_clusters.begin(),
                    plane_clusters.end()
                );
            }
        }

        for (int i = 0; i < this->config->us_n_stations; ++i) {
            for (auto &c : us_clusters[i]) {
                //std::cout << "US Station " << (i) << ": " << c.center << "\t" << c.radius << std::endl;
                detector->addHit(new HitData(c.center.X(), c.center.Y(), c.center.Z(), c.radius.X(), c.radius.Y(), c.radius.Z(), c.energy, c.time));
            }
        }

        detector->energyRange = snd::analysis_tools::FindRange(us_clusters);
        detector->timeRange = snd::analysis_tools::FindRange(us_clusters, true);

        // cluster DS
        //std::cout << "########################### DS #####################" << std::endl;
        std::vector<std::vector<snd::analysis_tools::Cluster>> ds_clusters(this->config->ds_n_stations);
        detector = new DetectorData("DS");
        toReturn->addDetector(detector);

        for (auto &p : this->dsPlanes) {
            int st = p.GetStation() - 1;

            if (st >= 0 && st < this->config->ds_n_stations+1) {
                //std::cout << p.GetHits().size() << " hits in DS plane " << st << std::endl;
                auto plane_clusters = ClustersPositions(*this->boundaries, p.GetHits(), this->config->ds_hor_spatial_resolution_y, this->config->ds_ver_spatial_resolution_x, 2);

                ds_clusters[st].insert(
                    ds_clusters[st].end(),
                    plane_clusters.begin(),
                    plane_clusters.end()
                );
            }
        }

        for (int i = 0; i < this->config->ds_n_stations; ++i) {
            for (auto &c : ds_clusters[i]) {
                detector->addHit(new HitData(c.center.X(), c.center.Y(), c.center.Z(), c.radius.X(), c.radius.Y(), c.radius.Z(), c.energy, c.time));
                //std::cout << "DS Station " << (i) << ": " << c.center << "\t" << c.radius << std::endl;
            }
        }

        std::vector<std::vector<snd::analysis_tools::Cluster>> generic_clusters;

        // US and SciFi are used to compute generic energy range

        generic_clusters.insert(
            generic_clusters.end(),
            scifi_clusters.begin(),
            scifi_clusters.end()
        );

        generic_clusters.insert(
            generic_clusters.end(),
            us_clusters.begin(),
            us_clusters.end()
        );

        toReturn->energyRange = snd::analysis_tools::FindRange(generic_clusters);

        // Consider aldo DS and Veto for time range

        generic_clusters.insert(
            generic_clusters.end(),
            ds_clusters.begin(),
            ds_clusters.end()
        );

        generic_clusters.insert(
            generic_clusters.end(),
            veto_clusters.begin(),
            veto_clusters.end()
        );

        toReturn->timeRange = snd::analysis_tools::FindRange(generic_clusters, true);

        return toReturn;
    }
}

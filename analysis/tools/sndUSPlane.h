#ifndef SND_USPLANE_H
#define SND_USPLANE_H

#include <vector>

#include "TVector3.h"
#include "TClonesArray.h"
#include "MuFilter.h"
#include "sndConfiguration.h"

namespace snd {
    namespace analysis_tools {
        class USPlane
        {
        public:
        // small and large SiPMs
        template <class T>
        struct sl_pair
        {
            T small{};
            T large{};
        };

        // right and left side of US bars
        template <class T>
        struct rl_pair
        {
            T right{};
            T left{};
        };

        // hits vector, each hit has info about timestamp, qdc and position
        struct USHit
        {
            int channel_index;
            int bar;

            double qdc;
            double timestamp;
            double x;   // position of right or left side of bar
            double y;
            double z;

            bool is_large;
            bool is_right;
        };

        USPlane(TClonesArray *snd_hits, Configuration configuration, MuFilter *muon_filter_geometry, int index_begin, int index_end, int station);

        const sl_pair<int> GetNHits() const;
        const int GetStation() const { return station_; };

        const sl_pair<double> GetTotQdc() const;
        const sl_pair<double> GetTotEnergy() const;
        const rl_pair<double> GetSideQdc() const;
        const rl_pair<double> GetBarQdc(int bar_to_compute) const;
        const sl_pair<int> GetBarNHits(int bar_to_compute) const;
        const std::vector<USHit> GetHits() const { return hits_; };
        // The centroid is the qdc-weighted mean of hit positions, considering only hits with positive qdc
        void FindCentroid();
        const TVector3 GetCentroid() const { return centroid_; };
        const TVector3 GetCentroidError() const { return centroid_error_; };

        void TimeFilter(double min_timestamp, double max_timestamp);

        private:
        std::vector<USHit> hits_;
        Configuration configuration_;
        TVector3 centroid_;
        TVector3 centroid_error_;
        int station_;
        };
    }
}

#endif
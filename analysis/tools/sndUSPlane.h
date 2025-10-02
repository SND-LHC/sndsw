#ifndef SND_USPLANE_H
#define SND_USPLANE_H

#include <vector>

#include "Math/Point3D.h"
#include "MuFilter.h"
#include "MuFilterHit.h"
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

        USPlane(std::vector<MuFilterHit*> snd_hits, Configuration configuration, MuFilter *muon_filter_geometry, int station);

        const sl_pair<int> GetNHits() const;
        const int GetStation() const { return station_; };

        const sl_pair<double> GetTotQdc() const;
        const sl_pair<double> GetTotEnergy() const;
        const rl_pair<double> GetSideQdc() const;
        const rl_pair<double> GetBarQdc(int bar_to_compute) const;
        const sl_pair<int> GetBarNHits(int bar_to_compute) const;
        const std::vector<USHit> GetHits() const { return hits_; };
        double HasShower() const { return static_cast<int>(hits_.size()) >= configuration_.us_min_n_hits_for_centroid; };
        // The centroid is the qdc-weighted mean of hit positions, considering only hits with positive qdc
        void FindCentroid();
        ROOT::Math::XYZPoint GetCentroid() const { return centroid_; };
        ROOT::Math::XYZPoint GetCentroidError() const { return centroid_error_; };

        void TimeFilter(double min_timestamp, double max_timestamp);

        private:
        std::vector<USHit> hits_;
        Configuration configuration_;
        ROOT::Math::XYZPoint centroid_;
        ROOT::Math::XYZPoint centroid_error_;
        int station_;
        };
    }
}

#endif
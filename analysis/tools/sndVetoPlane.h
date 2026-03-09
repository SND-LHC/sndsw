#ifndef SND_VETOPLANE_H
#define SND_VETOPLANE_H

#include <vector>

#include "MuFilter.h"
#include "MuFilterHit.h"
#include "sndConfiguration.h"

namespace snd {
    namespace analysis_tools {
        class VetoPlane
        {
        public:

        // right and left side of Veto bars
        template <class T>
        struct rl_pair
        {
            T right{};
            T left{};
        };

        // hits vector, each hit has info about timestamp, qdc and position
        struct VetoHit
        {
            int channel_index;
            int bar;

            double qdc;
            double timestamp;
            double x;
            double y;
            double z;

            bool is_x;  // true if vertical (measures x)
            bool is_right;

            void Print() const;
        };

        VetoPlane(std::vector<MuFilterHit*> snd_hits, const Configuration &configuration, MuFilter *muon_filter_geometry, int station);

        const int GetNHits() const { return hits_.size(); };
        const int GetStation() const { return station_; }

        const double GetTotQdc() const;
        const double GetBarQdc(int bar_to_compute) const;
        const int GetBarNHits(int bar_to_compute) const;
        const std::vector<VetoHit> GetHits() const { return hits_; };
        const int GetNHitBars() const;

        void TimeFilter(double min_timestamp, double max_timestamp);

        private:
        std::vector<VetoHit> hits_;
        Configuration configuration_;
        int station_;
        };
    }
}

#endif

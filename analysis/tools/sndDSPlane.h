#ifndef SND_DSPLANE_H
#define SND_DSPLANE_H

#include <vector>

#include "MuFilter.h"
#include "MuFilterHit.h"
#include "sndConfiguration.h"

namespace snd {
    namespace analysis_tools {
        class DSPlane
        {
        public:

        // right and left side of DS bars
        template <class T>
        struct rl_pair
        {
            T right{};
            T left{};
        };

        // hits vector, each hit has info about timestamp, qdc and position
        struct DSHit
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

        DSPlane(std::vector<MuFilterHit*> snd_hits, const Configuration &configuration, MuFilter *muon_filter_geometry, int station);

        const int GetNHits() const { return hits_.size(); };
        const int GetStation() const { return station_; }

        const double GetTotQdc() const;
        const std::vector<DSHit> GetHits() const { return hits_; };
        const int GetBarNHits(int bar_to_compute) const;

        void TimeFilter(double min_timestamp, double max_timestamp);
        const int GetNHitBars() const;

        private:
        std::vector<DSHit> hits_;
        Configuration configuration_;
        int station_;
        };
    }
}

#endif

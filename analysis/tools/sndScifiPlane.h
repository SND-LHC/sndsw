#ifndef SND_SCIFIPLANE_H
#define SND_SCIFIPLANE_H

#include <vector>

#include "TVector3.h"
#include "TClonesArray.h"
#include "Scifi.h"
#include "sndConfiguration.h"

namespace snd {
    namespace analysis_tools {
        class ScifiPlane
        {
        public:
            // x and y planes
            template <class T>
            struct xy_pair
            {
                T x{};
                T y{};
            };

            struct ScifiHit
            {
                double qdc{};
                double timestamp{};
                double x{};
                double y{};
                double z{};
                int channel_index{};
                bool is_x{};
            };

            ScifiPlane(TClonesArray *snd_hits, Configuration configuration, Scifi *scifi_geometry, int index_begin, int index_end, int station);

            const int GetStation() const { return station_; };
            const std::vector<ScifiHit> GetHits() const { return hits_; };
            const TVector3 GetCentroid() const { return centroid_; };
            const TVector3 GetCentroidError() const { return centroid_error_; }
            const xy_pair<double> GetTotQdc(bool only_positive = false) const;
            const xy_pair<double> GetTotEnergy(bool only_positive = false) const;
            const xy_pair<int> GetNHits() const;
            // Position of larger cluster of consecutive hits, allowing at most max_gap channels with no hit
            const TVector3 GetCluster(int max_gap) const;
            // The centroid is the qdc-weighted mean of hit positions, considering only hits with positive qdc
            void FindCentroid();
            bool IsShower() const;
            void TimeFilter(double min_timestamp, double max_timestamp);
            // qdc from hits within a given point and radius (square, not circle)
            xy_pair<double> GetPointQdc(const TVector3 &point, double radius) const;

        private:
            std::vector<ScifiHit> hits_;
            Configuration configuration_;
            TVector3 centroid_;
            TVector3 centroid_error_;

            int station_;
        };
    }
}

#endif
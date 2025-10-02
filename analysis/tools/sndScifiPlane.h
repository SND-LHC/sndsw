#ifndef SND_SCIFIPLANE_H
#define SND_SCIFIPLANE_H

#include <vector>

#include "Scifi.h"
#include "sndConfiguration.h"
#include "sndScifiHit.h"
#include "Math/Point3D.h"

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
                double timestamp{}; // timestamp is in clock cycles
                double x{};
                double y{};
                double z{};
                int channel_index{};
                bool is_x{};
            };

            ScifiPlane(std::vector<sndScifiHit*> snd_hits, Configuration configuration, Scifi *scifi_geometry, int station);

            const int GetStation() const { return station_; };
            const std::vector<ScifiHit> GetHits() const { return hits_; };
            const ROOT::Math::XYZPoint GetCentroid() const { return centroid_; };
            const ROOT::Math::XYZPoint GetCentroidError() const { return centroid_error_; }
            const xy_pair<double> GetTotQdc(bool only_positive = false) const;
            const xy_pair<double> GetTotEnergy(bool only_positive = false) const;
            const xy_pair<int> GetNHits() const;
            // Position of larger cluster of consecutive hits, allowing at most max_gap channels with no hit
            const ROOT::Math::XYZPoint GetCluster(int max_gap) const;
            // The centroid is the qdc-weighted mean of hit positions, considering only hits with positive qdc
            void FindCentroid();
            bool HasShower() const;
            void TimeFilter(double min_timestamp, double max_timestamp);
            // qdc from hits within a given point and radius (square, not circle)
            xy_pair<double> GetPointQdc(const ROOT::Math::XYZPoint &point, double radius) const;

        private:
            std::vector<ScifiHit> hits_;
            Configuration configuration_;
            ROOT::Math::XYZPoint centroid_;
            ROOT::Math::XYZPoint centroid_error_;

            int station_;
        };
    }
}

#endif
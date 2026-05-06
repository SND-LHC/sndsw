#ifndef SND_PLANETOOLS_H
#define SND_PLANETOOLS_H        

#include <vector>
#include <algorithm>
#include <numeric>
#include <type_traits>

#include "TClonesArray.h"
#include "Scifi.h"
#include "MuFilter.h"
#include "sndConfiguration.h"
#include "sndVetoPlane.h"
#include "sndScifiPlane.h"
#include "sndUSPlane.h"
#include "sndDSPlane.h"
#include "sndGeometryGetter.h"

namespace snd {
    namespace analysis_tools {
        // Produce veto, scifi, us and ds planes from data 
        std::vector<VetoPlane> FillVeto(const Configuration &configuration, TClonesArray *mufi_hits, MuFilter *mufilter_geometry);
        std::vector<ScifiPlane> FillScifi(const Configuration &configuration, TClonesArray *sf_hits, Scifi *scifi_geometry);
        std::vector<USPlane> FillUS(const Configuration &configuration, TClonesArray *mufi_hits, MuFilter *mufilter_geometry, bool use_small_sipms=false);
        std::vector<DSPlane> FillDS(const Configuration &configuration, TClonesArray *mufi_hits, MuFilter *mufilter_geometry);
    
        struct Cluster {
            ROOT::Math::XYZPoint center;
            ROOT::Math::XYZPoint radius;   // in 2D: circle radius, in 1D: half-length
            double energy{std::nan("")};    //GeV
            double time{std::nan("")};      //ns 
        };

        template <typename H>
        std::vector<Cluster> ClustersPositions(const snd::analysis_tools::DetectorBoundaries &boundaries, std::vector<H> hits, double min_radius_x, double min_radius_y, double max_gap = 1.0, double calibration_constant = 0.0) {

            static_assert(std::is_convertible_v<decltype(std::declval<const H>().HitPosition()), ROOT::Math::XYZPoint>,
                            "Hit must have HitPosition()");
            static_assert(std::is_invocable_v<decltype(&H::Print), const H&>, "Hit must have a Print() method");
            static_assert(std::is_same_v<decltype(std::declval<const H>().timestamp), double>, "Hit must have a double attribute 'timestamp'");
            static_assert(std::is_same_v<decltype(std::declval<const H>().qdc), double>, "Hit must have a double attribute 'qdc'");

            std::vector<snd::analysis_tools::Cluster> clusters;
            if (hits.empty()) return clusters;

            hits.erase(std::remove_if(hits.begin(), hits.end(), [](const auto& h) {
                if (std::isnan(h.z)) {
                    std::cout << "Removing the hit: ";
                    h.Print();
                    return true;
                }
                return false;
            }), hits.end());

                // Partition hits by type
            std::vector<H> xOnlyHits, yOnlyHits, xyHits;
            for (const auto& h : hits) {
                bool hasX = !std::isnan(h.x);
                bool hasY = !std::isnan(h.y);
                if      ( hasX && !hasY) xOnlyHits.push_back(h);
                else if (!hasX &&  hasY) yOnlyHits.push_back(h);
                else if ( hasX &&  hasY) xyHits.push_back(h);
            }

            // Lambda that sorts a group and clusters it along the given axis
            auto clusterAlong = [&](std::vector<H>& group, bool useX) -> std::vector<snd::analysis_tools::Cluster> {
                std::vector<snd::analysis_tools::Cluster> result;
                if (group.empty()) return result;

                std::sort(group.begin(), group.end(), [useX](const auto& a, const auto& b) {
                    float aPos = useX ? a.x : a.y;
                    float bPos = useX ? b.x : b.y;
                    if (std::isnan(aPos)) return false;
                    if (std::isnan(bPos)) return true;
                    return aPos < bPos;
                });

                size_t start{0}, N = group.size();
                while (start < N) {
                    size_t end = start;

                    while (end + 1 < N) {
                        float current_pos = useX ? group[end].x     : group[end].y;
                        float next_pos    = useX ? group[end + 1].x : group[end + 1].y;
                        if (std::abs(next_pos - current_pos) > max_gap) break;
                        ++end;
                    }

                    double sumx = 0.0, sumy = 0.0, sumz = 0.0;
                    double energy = 0.0, time = 0.0;
                    int countX = 0, countY = 0, countZ = 0;
                    double min_pos = 1e9, max_pos = -1e9;

                    for (size_t i = start; i <= end; ++i) {
                        auto p = group[i].HitPosition();

                        if (!std::isnan(p.X())) {
                            sumx += p.X();
                            countX++;
                        }
                        if (!std::isnan(p.Y())) {
                            sumy += p.Y();
                            countY++;
                        }
                        if (!std::isnan(p.Z())) {
                            sumz += p.Z();
                            energy += group[i].qdc;
                            time += group[i].timestamp;
                            countZ++;
                        }

                        double pos = useX ? p.X() : p.Y();
                        if (!std::isnan(pos)) {
                            min_pos = std::min(min_pos, pos);
                            max_pos = std::max(max_pos, pos);
                        }
                    }
                    
                    bool hasX = (countX > 0);
                    bool hasY = (countY > 0);
                    bool is2D = hasX && hasY;

                    double measured_radius = (hasX || hasY) ? (max_pos - min_pos) / 2.0 : std::nan("");
                    
                    auto boundary = snd::analysis_tools::FindBoundary(boundaries, sumz / countZ);
                    double x_avg = boundary->at("x_avg");
                    double x_min = boundary->at("x_min");
                    double x_max = boundary->at("x_max");
                    double y_avg = boundary->at("y_avg");
                    double y_min = boundary->at("y_min");
                    double y_max = boundary->at("y_max");
                    double z_min = boundary->at("z_min");
                    double z_max = boundary->at("z_max");

                    ROOT::Math::XYZPoint center(
                        hasX ? sumx / countX : x_avg,
                        hasY ? sumy / countY : y_avg,
                        sumz / countZ 
                    );

                    time /= countZ;
                    energy *= calibration_constant;

                    double rx = hasX ? std::max(measured_radius, min_radius_x) : ((x_max-x_min) / 2.0);
                    double ry = hasY ? std::max(measured_radius, min_radius_y) : ((y_max-y_min) / 2.0);
                    double rz = (z_max-z_min)/2.0;     //half of detector dimension 

                    ROOT::Math::XYZPoint radius(rx, ry, rz);
                    result.push_back({center, radius, energy, time});   
                    start = end + 1;
                }
                return result;
            };

                auto xOnlyClusters = clusterAlong(xOnlyHits, true);
                auto yOnlyClusters = clusterAlong(yOnlyHits, false);

                std::vector<snd::analysis_tools::Cluster> xyClusters;
                if (!xyHits.empty()) {
                    for (const auto& yc : clusterAlong(xyHits, false)) {
                        double y_lo = yc.center.Y() - yc.radius.Y();
                        double y_hi = yc.center.Y() + yc.radius.Y();
                        std::vector<H> sub;
                        for (const auto& h : xyHits)
                            if (h.HitPosition().Y() >= y_lo && h.HitPosition().Y() <= y_hi)
                                sub.push_back(h);
                        for (auto& c : clusterAlong(sub, true)) xyClusters.push_back(c);
                    }
                }


                if (xOnlyClusters.size() == 1 && yOnlyClusters.size() == 1 && xyClusters.empty()) {
                    const auto& xc = xOnlyClusters[0];
                    const auto& yc = yOnlyClusters[0];
                    ROOT::Math::XYZPoint center(xc.center.X(), yc.center.Y(), (xc.center.Z() + yc.center.Z()) / 2.0);
                    ROOT::Math::XYZPoint radius(xc.radius.X(), yc.radius.Y(), std::max(xc.radius.Z(), yc.radius.Z())); //anche se dovrebbero essere uguali
                    double energy = xc.energy+yc.energy;
                    double time = (xc.time + yc.time) / 2.0;
                    clusters.push_back({center, radius, energy, time});
                    return clusters;
                }


                    // Otherwise collect all clusters
                for (auto& c : xOnlyClusters) clusters.push_back(c);
                for (auto& c : yOnlyClusters) clusters.push_back(c);
                for (auto& c : xyClusters)    clusters.push_back(c);
                return clusters;
        }

    }
}

#endif

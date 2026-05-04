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
        };

        template <typename H>
        std::vector<Cluster> ClustersPositions(const snd::Configuration &configuration, const snd::analysis_tools::DetectorBoundaries &boundaries, std::vector<H> hits, double min_radius_x, double min_radius_y, double max_gap = 1.0) {

            static_assert(std::is_convertible_v<decltype(std::declval<const H>().HitPosition()), ROOT::Math::XYZPoint>,
                            "Hit class must have HitPosition()");
            static_assert(std::is_invocable_v<decltype(&H::Print), const H&>, "Hit class must have a Print() method");

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

            // Check if any hit has both x and y valid (2D mixed hits)
            bool any2D = std::any_of(hits.begin(), hits.end(), [](const auto& h) {
                return !std::isnan(h.x) && !std::isnan(h.y);
            });
            
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

                // std::cout << "DEBUG: sorted order:" << std::endl;
                // for (const auto& h :group) h.Print();

                size_t start{0};
                size_t N = group.size();
                while (start < N) {
                    size_t end = start;

                    while (end + 1 < N) {
                        float current_pos = useX ? group[end].x     : group[end].y;
                        float next_pos    = useX ? group[end + 1].x : group[end + 1].y;

                        if (std::abs(next_pos - current_pos) > max_gap) {
                                // std::cout << "DEBUG gap check: current_pos=" << current_pos 
                                // << " next_pos=" << next_pos 
                                // << " diff=" << std::abs(next_pos - current_pos)
                                // << " max_gap=" << max_gap << std::endl;
                                break;
                                }
    
                        ++end;
                    }

                    double sumx = 0.0, sumy = 0.0, sumz = 0.0;
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


                    double rx = hasX ? std::max(measured_radius, min_radius_x) : ((x_max-x_min) / 2.0);
                    double ry = hasY ? std::max(measured_radius, min_radius_y) : ((y_max-y_min) / 2.0);
                    double rz = (z_max-z_min)/2.0;     //half of detector dimension 

                    // std::cout << "DEBUG: New Cluster from hits " << start << " to " << end
                    //         << " (Total hits in cluster: " << (end - start + 1) << ")" << std::endl;
                    // for (size_t i = start; i <= end; ++i) group[i].Print();

                    ROOT::Math::XYZPoint radius(rx, ry, rz);
                    result.push_back({center, radius});   
                    start = end + 1;
                }
                return result;
            };

        if (any2D) {
            std::vector<snd::analysis_tools::Cluster> y_clusters = clusterAlong(hits, false);

            size_t hit_start = 0;
            for (const auto& yc : y_clusters) {
                // Collect all hits whose y falls within [cy - ry, cy + ry]
                double y_lo = yc.center.Y() - yc.radius.Y();
                double y_hi = yc.center.Y() + yc.radius.Y();

                std::vector<H> sub_hits;
                for (const auto& h : hits) {
                    auto p = h.HitPosition();
                    if (p.Y() >= y_lo && p.Y() <= y_hi)
                        sub_hits.push_back(h);
                }

                // Check if any hit in this y-cluster actually has an x coordinate
                bool hasX = std::any_of(sub_hits.begin(), sub_hits.end(), [](const auto& h) {
                    return !std::isnan(h.x);
                });

                if (hasX) {
                    // Subdivide along x — results replace the coarse y-cluster
                    auto x_sub = clusterAlong(sub_hits, true);
                    for (auto& c : x_sub) {
                        // Preserve the y and z information from the parent y-cluster
                        // for any component that x-clustering left as NaN
                        if (std::isnan(c.center.Y())) c.center.SetY(yc.center.Y());
                        if (std::isnan(c.radius.Y()))  c.radius.SetY(yc.radius.Y());
                        clusters.push_back(c);
                    }
                } else {
                    clusters.push_back(yc);
                }
            }
        } else {
                // Hits are strictly x-only or y-only — split and cluster independently
                std::vector<H> xHits, yHits;
                for (const auto& h : hits) {
                    if (!std::isnan(h.x)) xHits.push_back(h);
                    else                   yHits.push_back(h);
                }
                for (auto& c : clusterAlong(xHits, true))   clusters.push_back(c);
                for (auto& c : clusterAlong(yHits, false))  clusters.push_back(c);   
            }
            return clusters;
        }

    }
}

#endif

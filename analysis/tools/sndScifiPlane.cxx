#include "sndScifiPlane.h"

#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <vector>

#include "TVector3.h"
#include "Scifi.h"
#include "sndScifiHit.h"
#include "ShipUnit.h"
#include "Math/Point3D.h"

snd::analysis_tools::ScifiPlane::ScifiPlane(std::vector<sndScifiHit*> snd_hits, Configuration configuration, Scifi *scifi_geometry, int station) : configuration_(configuration), centroid_(std::nan(""), std::nan(""), std::nan("")), centroid_error_(std::nan(""), std::nan(""), std::nan("")), station_(station)
{
    for ( auto snd_hit : snd_hits)
    {
        ScifiHit hit;
        hit.channel_index = 512 * snd_hit->GetMat() + 128 * snd_hit->GetSiPM() + snd_hit->GetSiPMChan();
        hit.timestamp = (scifi_geometry->GetCorrectedTime(snd_hit->GetDetectorID(), snd_hit->GetTime(0)*ShipUnit::snd_TDC2ns, 0) / ShipUnit::snd_TDC2ns);  // timestamp is in clock cycles
        hit.qdc = snd_hit->GetSignal(0);
        hit.is_x = snd_hit->isVertical();

        TVector3 A, B;
        int detectorID = snd_hit->GetDetectorID();
        scifi_geometry->GetSiPMPosition(detectorID, A, B);
        hit.z = A.Z();
        if (hit.is_x)
        {
            hit.x = A.X();
            hit.y = std::nan("");
        }
        else
        {
            hit.x = std::nan("");
            hit.y = A.Y();
        }
        hits_.push_back(hit);
    }
}

const snd::analysis_tools::ScifiPlane::xy_pair<int> snd::analysis_tools::ScifiPlane::GetNHits() const
{
    xy_pair<int> counts{0, 0};
    counts.x = std::count_if(hits_.begin(), hits_.end(), [](auto &hit)
                             { return hit.is_x; });
    counts.y = static_cast<int>(hits_.size()) - counts.x;

    return counts;
}

bool snd::analysis_tools::ScifiPlane::HasShower() const
{
    if (configuration_.scifi_min_hits_in_window > configuration_.scifi_shower_window_width)
    {
        throw std::runtime_error{"min_hits > window_width"};
    }

    if (configuration_.scifi_shower_window_width > configuration_.scifi_n_channels_per_plane)
    {
        throw std::runtime_error{"window_width > n_channels_per_plane"};
    }

    xy_pair<std::vector<int>> is_hit;
    is_hit.x.resize(configuration_.scifi_n_channels_per_plane, 0);
    is_hit.y.resize(configuration_.scifi_n_channels_per_plane, 0);

    for (auto &hit : hits_)
    {
        if (hit.channel_index < 0 ||
            hit.channel_index >= configuration_.scifi_n_channels_per_plane)
        {
            throw std::out_of_range{"hit.channel_index out of range"};
        }
        (hit.is_x ? is_hit.x : is_hit.y)[hit.channel_index] = 1;
    }

    auto density = [&](const std::vector<int> &hit_arr)
    {
        int count{0};

        // Initial count for the first window
        for (int i{0}; i < configuration_.scifi_shower_window_width; ++i)
        {
            count += hit_arr[i];
        }

        if (count >= configuration_.scifi_min_hits_in_window)
            return true;

        // Slide the window across the array
        for (int i = configuration_.scifi_shower_window_width; i < configuration_.scifi_n_channels_per_plane; ++i)
        {
            count += hit_arr[i] - hit_arr[i - configuration_.scifi_shower_window_width];
            if (count >= configuration_.scifi_min_hits_in_window)
                return true;
        }

        return false;
    };

    return density(is_hit.x) && density(is_hit.y);
}

const ROOT::Math::XYZPoint snd::analysis_tools::ScifiPlane::GetCluster(int max_gap) const
{
    std::vector<double> pos_x(configuration_.scifi_n_channels_per_plane, std::nan(""));
    std::vector<double> pos_y(configuration_.scifi_n_channels_per_plane, std::nan(""));

    for (auto &hit : hits_)
    {
        if (hit.qdc > 0)
        {
            if (hit.is_x)
            {
                pos_x[hit.channel_index] = hit.x;
            }
            else
            {
                pos_y[hit.channel_index] = hit.y;
            }
        }
    }

    auto largest_cluster = [&](const std::vector<double> &positions)
    {
        int n = static_cast<int>(positions.size());

        int best_start = -1, best_end = -1, best_size = 0;
        int start = -1, gap_count = 0, size = 0;

        // Find the largest cluster
        for (int i = 0; i < n; ++i)
        {
            if (!std::isnan(positions[i]))
            {
                if (start == -1)
                    start = i; // Start a new cluster
                size++;
                gap_count = 0; // Reset consecutive gap counter
            }
            else
            {
                gap_count++;
                if (gap_count > max_gap)
                { // Too many consecutive gaps, finalize previous cluster
                    if (size > best_size)
                    {
                        best_start = start;
                        best_end = i - gap_count; // End before the excessive gaps
                        best_size = size;
                    }
                    // Reset for a new potential cluster
                    start = -1;
                    gap_count = 0;
                    size = 0;
                }
                else
                {
                    size++; // Gaps within max_gap still count in cluster size
                }
            }
        }

        // Check last cluster
        if (size > best_size)
        {
            best_start = start;
            best_end = n - 1;
            best_size = size;
        }

        // Compute the average of non-gap values in the best cluster
        if (best_start == -1 || best_end == -1)
            return std::nan(""); // No valid cluster found

        double sum = 0.0;
        int count = 0;
        for (int i = best_start; i <= best_end; ++i)
        {
            if (!std::isnan(positions[i]))
            {
                sum += positions[i];
                count++;
            }
        }

        return (count > 0) ? sum / count : std::nan("");
    };

    double cluster_x = largest_cluster(pos_x);
    double cluster_y = largest_cluster(pos_y);
    if (!(std::isnan(cluster_x) || std::isnan(cluster_y))) {
        return ROOT::Math::XYZPoint(cluster_x, cluster_y, hits_[0].z);
    }
    return ROOT::Math::XYZPoint(std::nan(""), std::nan(""), std::nan(""));
}

void snd::analysis_tools::ScifiPlane::TimeFilter(double min_timestamp, double max_timestamp)
{
    hits_.erase(std::remove_if(hits_.begin(), hits_.end(),
                               [&](auto &hit)
                               { return hit.timestamp < min_timestamp || hit.timestamp > max_timestamp; }),
                hits_.end());
}

snd::analysis_tools::ScifiPlane::xy_pair<double> snd::analysis_tools::ScifiPlane::GetPointQdc(const ROOT::Math::XYZPoint &point, double radius) const
{
    xy_pair<double> qdc{0.0, 0.0};
    for (const auto &hit : hits_) {
        if (hit.is_x && std::abs(hit.x - point.X()) <= radius) {
            qdc.x += hit.qdc;
        }
        else if (!hit.is_x && std::abs(hit.y - point.Y()) <= radius) {
            qdc.y += hit.qdc;
        }
    }
    return qdc;
}

void snd::analysis_tools::ScifiPlane::FindCentroid()
{
    centroid_.SetXYZ(0, 0, 0);
    double tot_qdc_x{0};
    double tot_qdc_y{0};
    std::vector<ScifiHit> cleaned_hits = hits_;

    cleaned_hits.erase(std::remove_if(cleaned_hits.begin(), cleaned_hits.end(),
                               [&](auto &hit)
                               { return hit.qdc <= 0; }),
                       cleaned_hits.end());
    int counts_x = std::count_if(cleaned_hits.begin(), cleaned_hits.end(), [](auto &hit)
                    { return hit.is_x; });
    int counts_y = static_cast<int>(cleaned_hits.size())-counts_x;
    if (counts_x < configuration_.scifi_min_n_hits_for_centroid && counts_y < configuration_.scifi_min_n_hits_for_centroid ) {
        centroid_ = ROOT::Math::XYZPoint(std::nan(""), std::nan(""), std::nan(""));
        return;
    }

    for (auto &hit : cleaned_hits)
    {
        if (hit.qdc > 0)
        {
            if (hit.is_x)
            {
                centroid_.SetX(centroid_.X() + hit.x * hit.qdc);
                tot_qdc_x += hit.qdc;
            }
            else
            {
                centroid_.SetY(centroid_.Y() + hit.y * hit.qdc);
                tot_qdc_y += hit.qdc;
            }

            centroid_.SetZ(centroid_.Z() + hit.z * hit.qdc);
        }
    }
    centroid_ = ROOT::Math::XYZPoint((tot_qdc_x > 0) ? centroid_.X() / tot_qdc_x : std::nan(""), (tot_qdc_y > 0) ? centroid_.Y() / tot_qdc_y : std::nan(""), (tot_qdc_x+tot_qdc_y > 0) ? centroid_.Z() / (tot_qdc_x+tot_qdc_y) : std::nan(""));
    centroid_error_ = ROOT::Math::XYZPoint(configuration_.scifi_centroid_error_x, configuration_.scifi_centroid_error_y, configuration_.scifi_centroid_error_z);
}

const snd::analysis_tools::ScifiPlane::xy_pair<double> snd::analysis_tools::ScifiPlane::GetTotQdc(bool only_positive) const
{
    xy_pair<double> qdc_sum{0.0, 0.0};
    qdc_sum.x = std::accumulate(hits_.begin(), hits_.end(), 0.0, [&](double current_sum, auto &hit)
                                { return (hit.is_x && (hit.qdc > 0 || !only_positive)) ? (current_sum + hit.qdc) : current_sum; });
    qdc_sum.y = std::accumulate(hits_.begin(), hits_.end(), 0.0, [&](double current_sum, auto &hit)
                                { return (!hit.is_x && (hit.qdc > 0 || !only_positive)) ? (current_sum + hit.qdc) : current_sum; });

    return qdc_sum;
}

const snd::analysis_tools::ScifiPlane::xy_pair<double> snd::analysis_tools::ScifiPlane::GetTotEnergy(bool only_positive) const
{
    xy_pair<double> energy{0.0, 0.0};
    auto qdc = GetTotQdc(only_positive);

    energy.x = qdc.x * configuration_.scifi_qdc_to_gev;
    energy.y = qdc.y * configuration_.scifi_qdc_to_gev;

    return energy;
}
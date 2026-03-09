#include "sndDSPlane.h"

#include <cmath>
#include <algorithm>
#include <numeric>
#include <vector>

#include "MuFilter.h"
#include "MuFilterHit.h"
#include "ShipUnit.h"
#include "FairLogger.h"

snd::analysis_tools::DSPlane::DSPlane(std::vector<MuFilterHit*> snd_hits, const Configuration &configuration, MuFilter *muon_filter_geometry, int station) : configuration_(configuration), station_(station)
{
    for ( auto mu_hit : snd_hits)
    {
        TVector3 A, B;
        int detectorID = mu_hit->GetDetectorID();
        muon_filter_geometry->GetPosition(detectorID, A, B);
        const int n_sipms  = muon_filter_geometry->GetnSiPMs(detectorID);
        const int n_sides = muon_filter_geometry->GetnSides(detectorID);
        for (int i{0}; i < n_sipms * n_sides; ++i)
        {
            if (mu_hit->isMasked(i) || mu_hit->GetSignal(i) < -990.) continue;
            DSHit hit;
            hit.bar = static_cast<int>(detectorID % 1000);
            hit.channel_index = n_sipms * n_sides * hit.bar + i;
            hit.is_right = (i >= n_sipms) ? true : false;
            hit.timestamp = configuration_.is_mc ? mu_hit->GetTime(i) / ShipUnit::snd_TDC2ns : mu_hit->GetTime(i);
            hit.qdc = mu_hit->GetSignal(i);
            // use the left and right measurements to calculate the x coordinate along the bar
            if (!mu_hit->isVertical()) {
                hit.is_x = false;
                float tmp_x = mu_hit->GetImpactXpos(true, true, false, configuration_.is_mc);
                hit.x = (tmp_x < -990.) ? std::nan("") : A.X() - tmp_x; 
                hit.y = A.Y();
            }
            else {
                hit.is_x = true;
                hit.x = A.X();
                hit.y = std::nan("");
            }
            hit.z = A.Z();
            hits_.push_back(hit);
        }
    }
}

const double snd::analysis_tools::DSPlane::GetTotQdc() const
{
    double tot_qdc = std::accumulate(hits_.begin(), hits_.end(), 0.0,
                               [](double sum, const auto &b) {return sum + b.qdc;});
    return tot_qdc;
}

const int snd::analysis_tools::DSPlane::GetBarNHits(int bar_to_compute) const
{
    int bar_hit = std::count_if(hits_.begin(), hits_.end(),
                                [bar_to_compute](const auto &hit) {return hit.bar == bar_to_compute;});
    return bar_hit;
}

void snd::analysis_tools::DSPlane::TimeFilter(double min_timestamp, double max_timestamp)
{
    hits_.erase(std::remove_if(hits_.begin(), hits_.end(),
                               [&](auto &hit)
                               { return hit.timestamp < min_timestamp || hit.timestamp > max_timestamp; }),
                hits_.end());
}

const int snd::analysis_tools::DSPlane::GetNHitBars() const
{
    int count{0};
    for (int bar{0}; bar < 2*configuration_.ds_bar_per_station; ++bar) {
        if (GetBarNHits(bar) > 0) count++;
    }
    return count;
}

void snd::analysis_tools::DSPlane::DSHit::Print() const 
{
    LOGF(INFO, "DSHit ch_idx :%d\tposition: (%f,%f,%f)\ttime: %f\tqdc: %f\tbar: %d\tis_x: %d\tis_right: %d", channel_index, x, y, z, timestamp, qdc, bar, is_x, is_right);
}
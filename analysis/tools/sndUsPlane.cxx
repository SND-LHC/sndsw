#include "sndUsPlane.h"

#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <vector>

#include "TClonesArray.h"
#include "TVector3.h"
#include "MuFilter.h"
#include "MuFilterHit.h"

snd::UsPlane::UsPlane(TClonesArray *snd_hits, snd::Configuration configuration, MuFilter *muon_filter_geometry, int index_begin, int index_end, int station) : configuration_(configuration), centroid_(std::nan(""), std::nan(""), std::nan("")), centroid_error_(std::nan(""), std::nan(""),std::nan("")), station_(station)
{
    if (index_begin > index_end)
    {
        throw std::runtime_error{"Begin index > end index"};
    }
    for (int j{index_begin}; j < index_end; ++j)
    {
        auto mu_hit = static_cast<MuFilterHit *>(snd_hits->At(j));
        
        for (int i{0}; i < 16; ++i)
        {
            if (mu_hit->isMasked(i) || mu_hit->GetSignal(i) < -990.) continue;
            UsHit hit;
            hit.bar = static_cast<int>(mu_hit->GetDetectorID() % 1000);
            hit.channel_index = 16 * hit.bar + i;
            hit.timestamp = mu_hit->GetTime(i);
            hit.qdc = mu_hit->GetSignal(i);
            hit.is_large = !mu_hit->isShort(i);
        
            TVector3 A, B;
            int detectorID = mu_hit->GetDetectorID();
            muon_filter_geometry->GetPosition(detectorID, A, B);
            hit.is_right = i > 7 ? true : false;
            hit.x = hit.is_right ? B.X() : A.X();
            hit.y = A.Y();
            hit.z = A.Z();
            hits_.push_back(hit);
        }
    }
}

void snd::UsPlane::FindCentroid()
{
    // min number of hit in the plane to attempt to find a centroid
    if (hits_.size() < configuration_.us_min_n_hits_for_centroid)
    {
        // std::cout<<"Not enough hits in US plane " << station_ <<" to find centroid\n";
        return;
    }
    
    double total_qdc = GetTotQdc().l;
    if (total_qdc > 0)
    {
        for (const auto &hit : hits_)
        {
            // weigthed sum for y and z
            double  weighted_sum_x{0.0}, weighted_sum_y{0.0}, weighted_sum_z{0.0};
            double total_qdc{0.0};
            for (const auto &hit : hits_)
            {
                if (hit.qdc > 0)
                {
                    weighted_sum_x += hit.x * hit.qdc;
                    weighted_sum_y += hit.y * hit.qdc;
                    weighted_sum_z += hit.z * hit.qdc;
                    total_qdc += hit.qdc;
                }
            }
            double x = weighted_sum_x / total_qdc;
            double y = weighted_sum_y / total_qdc;
            double z = weighted_sum_z / total_qdc;
            centroid_.SetXYZ(x, y, z);
        }
    }
    centroid_error_.SetXYZ(configuration_.us_centroid_error_x, configuration_.us_centroid_error_y, configuration_.us_centroid_error_z);
}

const snd::UsPlane::sl_pair<double> snd::UsPlane::GetTotQdc() const
{
    sl_pair<double> totQdc{0.0, 0.0};
    for (const auto &hit : hits_)
    {
        if (hit.is_large)
            totQdc.l += hit.qdc;
        else
            totQdc.s += hit.qdc;
    }
    return totQdc;
}

const snd::UsPlane::sl_pair<double> snd::UsPlane::GetTotEnergy() const
{
    sl_pair<double> tot_energy{0.0, 0.0};
    sl_pair<double> tot_qdc = GetTotQdc();

    tot_energy.l = tot_qdc.l *configuration_.us_qdc_to_gev; 
    tot_energy.s = tot_qdc.s *configuration_.us_qdc_to_gev;   //ask Guido

    return tot_energy;
}


const snd::UsPlane::rl_pair<double> snd::UsPlane::GetSideQdc() const
{
    rl_pair<double> side_qdc{0.0, 0.0};
    for (const auto &hit : hits_)
    {
        if (hit.is_large)
        {
            if (hit.is_right)
                side_qdc.r += hit.qdc;
            else
                side_qdc.l += hit.qdc;
        }
    }
    return side_qdc;
}

const snd::UsPlane::rl_pair<double> snd::UsPlane::GetBarQdc(int bar_to_compute) const
{
    rl_pair<double> bar_qdc{0.0, 0.0};
    for (const auto &hit : hits_)
    {
        if (hit.bar != bar_to_compute)
            continue;
        else
        {
            if (hit.is_large)
            {
                if (hit.is_right)
                    bar_qdc.r += hit.qdc;
                else
                    bar_qdc.l += hit.qdc;
            }
        }
    }
    return bar_qdc;
}

const snd::UsPlane::sl_pair<int> snd::UsPlane::GetBarNHits(int bar_to_compute) const
{
    sl_pair<int> bar_hit{0, 0};
    for (const auto &hit : hits_)
    {
        if (hit.bar != bar_to_compute)
            continue;
        else
        {
            if (hit.is_large)
            
                    bar_hit.l++;
            else
                    bar_hit.s++;
            
        }
    }
    return bar_hit;
}

void snd::UsPlane::TimeFilter(double min_timestamp, double max_timestamp)
{
    hits_.erase(std::remove_if(hits_.begin(), hits_.end(),
                               [&](auto &hit)
                               { return hit.timestamp < min_timestamp || hit.timestamp > max_timestamp; }),
                hits_.end());
}

const snd::UsPlane::sl_pair<int> snd::UsPlane::GetNHits() const
{
    sl_pair<int> counts{0, 0};
    counts.l = std::count_if(hits_.begin(), hits_.end(), [](auto &hit)
                             { return hit.is_large; });
    counts.s = hits_.size() - counts.l;

    return counts;
}
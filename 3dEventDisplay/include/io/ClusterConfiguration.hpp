/**
 * @file ClusterConfiguration.hpp
 * @brief Defines cluster configuration parameters for detector data.
 *
 * ClusterConfiguration encapsulates the hit clustering parameters for each detector
 * type (Veto, SciFi, US, DS). Each detector has configurable gap thresholds and
 * minimum hit counts per cluster, which control how raw hits are grouped into
 * logical clusters for event reconstruction and visualization.
 *
 * @author Enrico Bartocetti
 * @date 2026-05-18
 */

#pragma once

namespace snd3D {
    struct ClusterConfiguration {
        double vetoMaxGap;
        int vetoMinHitInCluster;
        double sciFiMaxGap;
        int sciFiMinHitInCluster;
        double usMaxGap;
        int usMinHitInCluster;
        double dsMaxGap;
        int dsMinHitInCluster;

        ClusterConfiguration(
            double _vetoMaxGap,
            int _vetoMinHitInCluster,
            double _sciFiMaxGap,
            int _sciFiMinHitInCluster,
            double _usMaxGap,
            int _usMinHitInCluster,
            double _dsMaxGap,
            int _dsMinHitInCluster
        ) : vetoMaxGap(_vetoMaxGap),
            vetoMinHitInCluster(_vetoMinHitInCluster),
            sciFiMaxGap(_sciFiMaxGap),
            sciFiMinHitInCluster(_sciFiMinHitInCluster),
            usMaxGap(_usMaxGap),
            usMinHitInCluster(_usMinHitInCluster),
            dsMaxGap(_dsMaxGap),
            dsMinHitInCluster(_dsMinHitInCluster)
        {}
    };
}


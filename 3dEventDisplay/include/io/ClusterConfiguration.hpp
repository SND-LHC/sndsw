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


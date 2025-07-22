#ifndef SND_CONFIGURATION_H
#define SND_CONFIGURATION_H

#include "Scifi.h"
#include "MuFilter.h"

namespace snd {
    struct Configuration
    {
    enum class Option
    {
        ti18_2024_2025,
        ti18_2022_2023,
        test_beam_2023, 
        test_beam_2024
    };
    
    double scifi_qdc_to_gev;
    double scifi_fiber_lenght;
    double scifi_centroid_error_x;
    double scifi_centroid_error_y;
    double scifi_centroid_error_z;
    double scifi_min_timestamp;
    double scifi_max_timestamp;
    // For plots
    double scifi_x_min;
    double scifi_x_max;
    double scifi_y_min;
    double scifi_y_max;
    double scifi_z_min;
    double scifi_z_max;

    double us_qdc_to_gev;
    double us_centroid_error_x;
    double us_centroid_error_y;
    double us_centroid_error_z;
    double us_min_timestamp;
    double us_max_timestamp;
    // For plots
    double us_x_min;
    double us_x_max;
    double us_y_min;
    double us_y_max;
    double us_z_min;
    double us_z_max;

    double ds_hor_spatial_resolution_x;
    double ds_hor_spatial_resolution_y;
    double ds_hor_spatial_resolution_z;
    double ds_ver_spatial_resolution_x;
    double ds_ver_spatial_resolution_y;
    double ds_ver_spatial_resolution_z;

    int veto_n_stations;
    
    int scifi_n_stations;
    int scifi_n_channels_per_plane;
    int scifi_boards_per_plane;
    int scifi_shower_window_width;
    int scifi_min_hits_in_window;
    int scifi_min_n_hits_for_centroid;

    int us_n_stations;
    int us_bar_per_station;
    int us_n_channels_per_station;
    int us_n_sipm_per_bar;
    int us_min_n_hits_for_centroid;

    int centroid_min_valid_station;

    int ds_n_stations;

    Configuration(Option option, Scifi *scifi_geometry, MuFilter *muon_filter_geometry);
    static Option GetOption(int run_number);
    };
}

#endif
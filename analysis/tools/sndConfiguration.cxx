#include "sndConfiguration.h"

#include <cmath>
#include <iostream>
#include <stdexcept>

snd::Configuration::Configuration(Option option)
{
    // Common parameters (for now)
    scifi_centroid_error_x = 0.025;
    scifi_centroid_error_y = 0.025;
    scifi_centroid_error_z = 0.025 * 6;
    scifi_min_timestamp = -0.5;
    scifi_max_timestamp = 0.5;

    us_bar_per_station = 10;
    us_n_channels_per_station = 160;
    us_n_sipm_per_bar = 16;
    us_min_n_hits_for_centroid = 15;
    us_qdc_to_gev = 0.0151;
    us_centroid_error_x = 40.0 / std::sqrt(12);
    us_centroid_error_y = 6.0 / std::sqrt(12);
    us_centroid_error_z = 1.0;
    us_min_timestamp = -0.5;
    us_max_timestamp = 3.0;

    ds_spatial_resolution = 1.0/std::sqrt(12);

    if (option == Option::ti18_2024_2025)
    {
        veto_n_stations = 3;

        scifi_n_stations = 5;
        scifi_n_channels_per_plane = 512 * 3;
        scifi_boards_per_plane = 3;
        scifi_shower_window_width = 128;
        scifi_min_hits_in_window = 10;
        scifi_min_n_hits_for_centroid = 5;
        scifi_qdc_to_gev = 0.14;
        scifi_x_min = -50.0;
        scifi_x_max = 0.0;
        scifi_y_min = 10.0;
        scifi_y_max = 60.0;
        scifi_z_min = 280.0;
        scifi_z_max = 360.0;

        us_n_stations = 5;
        us_x_min = -80.0;
        us_x_max = 0.0;
        us_y_min = 0.0;
        us_y_max = 80.0;
        us_z_min = 370.0;
        us_z_max = 480.0;

        centroid_min_valid_station = 2;
        
        ds_n_stations = 4;
    }
    
    else if (option == Option::ti18_2022_2023)
    {
        veto_n_stations = 2;

        scifi_n_stations = 5;
        scifi_n_channels_per_plane = 512 * 3;
        scifi_boards_per_plane = 3;
        scifi_shower_window_width = 128;
        scifi_min_hits_in_window = 10;
        scifi_min_n_hits_for_centroid = 5;
        scifi_qdc_to_gev = 0.14;
        scifi_x_min = -50.0;
        scifi_x_max = 0.0;
        scifi_y_min = 10.0;
        scifi_y_max = 60.0;
        scifi_z_min = 280.0;
        scifi_z_max = 360.0;
        scifi_centroid_error_x = 0.025;
        scifi_centroid_error_y = 0.025;
        scifi_centroid_error_z = 0.025 * 6;
        scifi_min_timestamp = -0.5;
        scifi_max_timestamp = 0.5;

        us_n_stations = 5;
        us_x_min = -80.0;
        us_x_max = 0.0;
        us_y_min = 0.0;
        us_y_max = 80.0;
        us_z_min = 370.0;
        us_z_max = 480.0;

        centroid_min_valid_station = 2;

        ds_n_stations = 4;

    }
    
    else if (option == Option::test_beam_2023)
    {   
        veto_n_stations = 0;
        scifi_n_stations = 4;
        scifi_n_channels_per_plane = 512;
        scifi_boards_per_plane = 1;
        scifi_shower_window_width = 128;
        scifi_min_hits_in_window = 36;
        scifi_min_n_hits_for_centroid = 0;
        scifi_qdc_to_gev = 0.053;
        scifi_x_min = -47.0;
        scifi_x_max = -27.0;
        scifi_y_min = 35.0;
        scifi_y_max = 55.0;
        scifi_z_min = 310.0;
        scifi_z_max = 360.0;

        us_n_stations = 5;
        us_x_min = -80.0;
        us_x_max = 5.0;
        us_y_min = 10.0;
        us_y_max = 80.0;
        us_z_min = 370.0;
        us_z_max = 480.0;

        centroid_min_valid_station = 0;

        ds_n_stations = 1;

    }
    else if (option == Option::test_beam_2024)
    {   
        veto_n_stations = 0;
        scifi_n_stations = 4;
        scifi_n_channels_per_plane = 512;
        scifi_boards_per_plane = 1;
        scifi_shower_window_width = 128;
        scifi_min_hits_in_window = 36;
        scifi_min_n_hits_for_centroid = 0;
        scifi_qdc_to_gev = 0.14;
        scifi_x_min = -47.0;
        scifi_x_max = -27.0;
        scifi_y_min = 35.0;
        scifi_y_max = 55.0;
        scifi_z_min = 310.0;
        scifi_z_max = 380.0;

        us_n_stations = 0;
        us_x_min = std::nan("");
        us_x_max = std::nan("");
        us_y_min = std::nan("");
        us_y_max = std::nan("");
        us_z_min = std::nan("");
        us_z_max = std::nan("");

        centroid_min_valid_station = 0;

        ds_n_stations = 1;

    }
    else
    {
        throw std::invalid_argument("Unknown configuration option");
    }
}

snd::Configuration snd::Configuration::ChooseConfiguration(int run_number)
{
    if (run_number >= 100840) {
        std::cout << "Using configuration  >>>>>>>>>>\t test_beam_2024 \t<<<<<<<<<<" <<std::endl;
        return snd::Configuration(snd::Configuration::Option::test_beam_2024);
    }
    if (run_number >= 100000) {
        std::cout << "Using configuration  >>>>>>>>>>\t test_beam_2023 \t<<<<<<<<<<" <<std::endl;
        return snd::Configuration(snd::Configuration::Option::test_beam_2023);
    }
    else if (run_number < 7648) {
        std::cout << "Using configuration  >>>>>>>>>>\t ti18_22_23 \t<<<<<<<<<<" <<std::endl;
        return snd::Configuration(snd::Configuration::Option::ti18_2022_2023);
    }
    else {
        std::cout << "Using configuration  >>>>>>>>>>\t ti18 \t<<<<<<<<<<" <<std::endl;
        return snd::Configuration(snd::Configuration::Option::ti18_2024_2025);
    }
}
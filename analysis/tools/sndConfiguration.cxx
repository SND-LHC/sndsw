#include "sndConfiguration.h"

#include <cmath>
#include <iostream>
#include <stdexcept>

#include "Scifi.h"
#include "MuFilter.h"

snd::Configuration::Configuration(Option option, Scifi *scifi_geometry, MuFilter *muon_filter_geometry)
{
    // Parameters from geometry
    scifi_n_stations = scifi_geometry->GetConfParI("Scifi/nscifi");
    scifi_boards_per_plane = scifi_geometry->GetConfParI("Scifi/nmats");
    scifi_n_channels_per_plane = scifi_geometry->GetConfParI("Scifi/nsipm_channels") * scifi_geometry->GetConfParI("Scifi/nmats") * scifi_geometry->GetConfParI("Scifi/nsipm_mat");
    scifi_fiber_lenght = scifi_geometry->GetConfParF("Scifi/fiber_length");
    scifi_centroid_error_x = scifi_geometry->GetConfParF("Scifi/channel_width");
    scifi_centroid_error_y = scifi_geometry->GetConfParF("Scifi/channel_width");
    scifi_centroid_error_z = scifi_geometry->GetConfParF("Scifi/channel_height");

    veto_n_stations = muon_filter_geometry->GetConfParI("MuFilter/NVetoPlanes");

    us_n_stations = muon_filter_geometry->GetConfParI("MuFilter/NUpstreamPlanes");
    us_bar_per_station = muon_filter_geometry->GetConfParI("MuFilter/NUpstreamBars");
    us_n_sipm_per_bar = muon_filter_geometry->GetConfParI("MuFilter/UpstreamnSiPMs") * muon_filter_geometry->GetConfParI("MuFilter/UpstreamnSides");
    us_n_channels_per_station = us_bar_per_station * us_n_sipm_per_bar;
    us_centroid_error_x = muon_filter_geometry->GetConfParF("MuFilter/UpstreamBarX") / std::sqrt(12);
    us_centroid_error_y = muon_filter_geometry->GetConfParF("MuFilter/UpstreamBarY") / std::sqrt(12);
    us_centroid_error_z = muon_filter_geometry->GetConfParF("MuFilter/UpstreamBarZ");

    ds_n_stations = muon_filter_geometry->GetConfParI("MuFilter/NDownstreamPlanes");
    ds_hor_spatial_resolution_x = muon_filter_geometry->GetConfParF("MuFilter/DownstreamBarX") / std::sqrt(12);
    ds_hor_spatial_resolution_y = muon_filter_geometry->GetConfParF("MuFilter/DownstreamBarY") / std::sqrt(12);
    ds_hor_spatial_resolution_z = muon_filter_geometry->GetConfParF("MuFilter/DownstreamBarZ") / std::sqrt(12);
    ds_ver_spatial_resolution_x = muon_filter_geometry->GetConfParF("MuFilter/DownstreamBarX_ver") / std::sqrt(12);
    ds_ver_spatial_resolution_y = muon_filter_geometry->GetConfParF("MuFilter/DownstreamBarY_ver") / std::sqrt(12);
    ds_ver_spatial_resolution_z = muon_filter_geometry->GetConfParF("MuFilter/DownstreamBarZ_ver") / std::sqrt(12);

    // Common parameters not present in geometry
    scifi_min_timestamp = -0.5;
    scifi_max_timestamp = 0.5;
    us_min_timestamp = -0.5;
    us_max_timestamp = 3.0;
    us_min_n_hits_for_centroid = 15;
    us_qdc_to_gev = 0.0151;

    // Ad hoc parameters not present in geometry
    if (option == Option::ti18_2024_2025)
    {
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
        
        us_x_min = -80.0;
        us_x_max = 0.0;
        us_y_min = 0.0;
        us_y_max = 80.0;
        us_z_min = 370.0;
        us_z_max = 480.0;

        centroid_min_valid_station = 2;
    }
    
    else if (option == Option::ti18_2022_2023)
    {
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
        scifi_min_timestamp = -0.5;
        scifi_max_timestamp = 0.5;

        us_x_min = -80.0;
        us_x_max = 0.0;
        us_y_min = 0.0;
        us_y_max = 80.0;
        us_z_min = 370.0;
        us_z_max = 480.0;

        centroid_min_valid_station = 2;
    }
    
    else if (option == Option::test_beam_2023)
    {   
        scifi_shower_window_width = 128;
        scifi_min_hits_in_window = 36;
        scifi_min_n_hits_for_centroid = 0;
        scifi_qdc_to_gev = 0.053; // mirrors

        scifi_x_min = -47.0;
        scifi_x_max = -27.0;
        scifi_y_min = 35.0;
        scifi_y_max = 55.0;
        scifi_z_min = 310.0;
        scifi_z_max = 360.0;

        us_x_min = -80.0;
        us_x_max = 5.0;
        us_y_min = 10.0;
        us_y_max = 80.0;
        us_z_min = 370.0;
        us_z_max = 480.0;

        centroid_min_valid_station = 0;
    }
    else if (option == Option::test_beam_2024)
    {   
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

        us_x_min = std::nan("");
        us_x_max = std::nan("");
        us_y_min = std::nan("");
        us_y_max = std::nan("");
        us_z_min = std::nan("");
        us_z_max = std::nan("");

        centroid_min_valid_station = 0;
    }
    else
    {
        throw std::invalid_argument("Unknown configuration option");
    }
}

snd::Configuration::Option snd::Configuration::GetOption(int run_number)
{
    if (run_number >= 100840) {
        std::cout << "Choosing option  >>>>>>>>>>\t test_beam_2024 \t<<<<<<<<<<" <<std::endl;
        return snd::Configuration::Option::test_beam_2024;
    }
    if (run_number >= 100000) {
        std::cout << "Choosing option  >>>>>>>>>>\t test_beam_2023 \t<<<<<<<<<<" <<std::endl;
        return snd::Configuration::Option::test_beam_2023;
    }
    else if (run_number < 7648) {
        std::cout << "Choosing option  >>>>>>>>>>\t ti18_2022_2023 \t<<<<<<<<<<" <<std::endl;
        return snd::Configuration::Option::ti18_2022_2023;
    }
    else {
        std::cout << "Choosing option  >>>>>>>>>>\t ti18_2024_2025 \t<<<<<<<<<<" <<std::endl;
        return snd::Configuration::Option::ti18_2024_2025;
    }
}
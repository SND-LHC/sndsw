#pragma once

#include <string_view>
#include <cstdint>

namespace snd3D::constants {

    namespace limits {
        inline constexpr float PROJ_FARPLANE = 2000;
        inline constexpr float PROJ_NEARPLANE = 0.1f;
        inline constexpr float PROJ_FOVY_MAX = 115;
        inline constexpr float PROJ_FOVY_MIN = 45;
        inline constexpr float ZOOM_DISTANCE_MIN = 5;
        inline constexpr float GUI_FONT_SIZE_MIN = 0.5f;
        inline constexpr float ALPHA_VALUE_MIN = 0;
        inline constexpr float ALPHA_VALUE_MAX = 1;
        inline constexpr float EDGE_THICKNESS_MIN = 0.5f;
        inline constexpr float EDGE_THICKNESS_MAX = 5;
    }

    namespace defaults {
        inline constexpr float PROJ_FOVY = 80;
        inline constexpr bool SHOW_PIVOT = true;
        inline constexpr bool SHOW_AXIS = true;
        inline constexpr bool TRANSPARENCY = true;
        inline constexpr float EDGE_ALPHA_VALUE = 0.15f;
        inline constexpr float FACE_ALPHA_VALUE = 0.03f;
        inline constexpr float EDGE_THICKNESS = 1.5f;
        inline constexpr bool ORTHOGRAPHIC_PROJECTION = false;
        inline constexpr int AXIS_WIDGET_SIZE = 100;
        inline constexpr int AXIS_WIDGET_MARGIN = 20;

        namespace colors {
            inline constexpr float BACKGROUND_R = 0;
            inline constexpr float BACKGROUND_G = 0;
            inline constexpr float BACKGROUND_B = 0;
            inline constexpr float PIVOT_R = 0;
            inline constexpr float PIVOT_G = 1;
            inline constexpr float PIVOT_B = 1;
            inline constexpr float HIT_ENERGY_MIN_R = 0;
            inline constexpr float HIT_ENERGY_MIN_G = 1;
            inline constexpr float HIT_ENERGY_MIN_B = 1;
            inline constexpr float HIT_ENERGY_MAX_R = 1;
            inline constexpr float HIT_ENERGY_MAX_G = 1;
            inline constexpr float HIT_ENERGY_MAX_B = 0;
        }

        namespace window {
            inline constexpr int WIDTH = 1280;
            inline constexpr int HEIGHT = 720;
            inline constexpr const char* NAME = "SND 3D Event Display";
            inline constexpr bool VSYNC = true;
        }
    }

    namespace paths {
        inline constexpr std::string_view SHADER = "assets/shaders/";
        inline constexpr std::string_view SCREENSHOT = "output/";
        inline constexpr std::string_view SND_LOGO = "assets/img/SND_Logo.png";
        inline constexpr std::string_view GEOMETRIES = "assets/geometries/";
    }

    namespace factors {
        inline constexpr float GUI_FONT_RESIZE = 1.1f;
        inline constexpr float ZOOM = 0.1f;
        inline constexpr float ROTATION_SPEED = 2;
        inline constexpr float PAN = 20;
    }

    namespace sizes {
        inline constexpr float GUI_FONT = 0.8f;
        inline constexpr float FONT = 30;
        inline constexpr int ANCHOR = 15;
        inline constexpr float PIVOT = 3;
        inline constexpr int PADDING = 10;
        inline constexpr float AXIS = 2.5f;
        inline constexpr float COLOR_SCALE_WIDTH = 300;
        inline constexpr float COLOR_SCALE_HEIGHT = 20;
    }

    namespace graphics {
        inline constexpr uint8_t VERTICES_LAYER = 0;
        inline constexpr uint8_t COLORS_LAYER = 1;
        inline constexpr uint8_t NORMALS_LAYER = 2;
        inline constexpr uint8_t TEXTURE_LAYER = 3;
    }

    namespace data {
        inline constexpr int MIN_SCIFI_ENTRIES = 20;
        inline constexpr int MIN_US_ENTRIES = 50;
    }
}

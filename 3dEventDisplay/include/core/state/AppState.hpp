#pragma once

namespace snd3D {
    enum class AppState {
        // INITIALIZATION
        RUN_CHOICE,
        RUN_LOAD,
        EVENT_CHOICE,
        EVENT_LOAD,
        ROOT_GEOMETRY_LOAD,
        DEFAULT_GEOMETRY_LOAD,
        GEOMETRY_LOAD_FAILED,
        USER_GEOMETRY_CHOICE,
        USER_GEOMETRY_LOAD,
        
        SHOW_LOADING,
        ERROR,

        // RUNTIME UPDATES
        CHANGE_GEOMETRY_START,
        CHANGE_GEOMETRY_BROWSE,
        CHANGE_GEOMETRY_LOAD,
        CHANGE_EVENT_CHOICE,
        CHANGE_EVENT_LOAD,
        CHANGE_RUN_CHOICE,
        CHANGE_RUN_LOAD,

        // INTERACTION
        INTERACTION,
        EXPORT_IMAGE,

        CLOSED
    };

    inline const char* appStateToString(AppState state) {
        switch (state) {
            case AppState::RUN_CHOICE:              return "Choosing run";
            case AppState::RUN_LOAD:                return "Loading run";
            case AppState::EVENT_CHOICE:            return "Choosing event";
            case AppState::EVENT_LOAD:              return "Loading event";
            case AppState::ROOT_GEOMETRY_LOAD:      return "Loading root geometry";
            case AppState::DEFAULT_GEOMETRY_LOAD:   return "Loading default geometry";
            case AppState::GEOMETRY_LOAD_FAILED:    return "Geometry file load failed";
            case AppState::USER_GEOMETRY_CHOICE:    return "Choosing geometry";
            case AppState::USER_GEOMETRY_LOAD:      return "Loading user geometry";
            case AppState::SHOW_LOADING:            return "Loading data";
            case AppState::ERROR:                   return "Error";
            case AppState::CHANGE_GEOMETRY_START:   return "Opening new geometry";
            case AppState::CHANGE_GEOMETRY_BROWSE:  return "Choosing new geometry";
            case AppState::CHANGE_GEOMETRY_LOAD:    return "Loading new geometry";
            case AppState::CHANGE_EVENT_CHOICE:     return "Choosing new event";
            case AppState::CHANGE_EVENT_LOAD:       return "Loading new event";
            case AppState::CHANGE_RUN_CHOICE:       return "Choosing new run";
            case AppState::CHANGE_RUN_LOAD:         return "Loading new run";
            case AppState::INTERACTION:             return "Interaction";
            case AppState::EXPORT_IMAGE:            return "Export image";
            case AppState::CLOSED:                  return "Closing";
            default:                                return "Unknown";
        }
    }
}

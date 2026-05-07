#pragma once

#include <string>
#include <memory>
#include <cstdint>
#include <stack>

#include "core/state/AppState.hpp"
#include "io/RunData.hpp"
#include "io/EventData.hpp"

namespace snd3D {
    class AppStateManager {
        public:
            void update();
            void close();

            AppState getCurrentState();
            std::string getMessage();
            const RunData* getRun();
            const EventData* getEvent();

            // INITIALIZATION
            void numberSelected(int64_t number);
            int64_t getPendingNumber();
            void runLoaded(RunData* runData);
            void eventLoaded(EventData* eventData);
            void openGeometryDialog();
            void geometryFileSelected(std::string filePath);
            void resetDefaultGeometry();
            std::string getDetectorPath();
            void geometryLoaded();
            void errorInitializing(std::string exceptionMessage = "");
            void previousStep();

            // RUNTIME CHANGES
            void startRunChange();
            void startEventChange();
            void changeEvent(int64_t offset);

            // INTERACTION
            void toggleImageExport();

        private:
            AppState currentState = AppState::RUN_CHOICE;
            AppState nextState = AppState::RUN_CHOICE;
            std::stack<AppState> statesHistory;

            int64_t pendingNumber;
            std::unique_ptr<RunData> run;
            std::unique_ptr<EventData> event;
            std::string detectorPath;
            std::string message;

            void setNextStateFromHistory();
    };
}

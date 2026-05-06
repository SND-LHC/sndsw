#include "core/state/AppStateManager.hpp"

#include <iostream>

#include "core/Constants.hpp"

namespace snd3D {

    void AppStateManager::update() {
        if (this->currentState != this->nextState) {
            this->currentState = this->nextState;
        }

        // Once the GUI showed the loading message, start loading the data
        switch (this->currentState) {
            case AppState::SHOW_LOADING:
                this->setNextStateFromHistory();
                break;

            case AppState::CHANGE_GEOMETRY_START:
                this->nextState = AppState::CHANGE_GEOMETRY_BROWSE;
                this->message = "Opening File Browser...";
                break;

            case AppState::CHANGE_EVENT_START:
                this->nextState = AppState::SHOW_LOADING;
                this->message = "Loading new EVENT:\n" + std::to_string(this->pendingNumber) + " - RUN N° " + std::to_string(this->run->runNumber);
                this->statesHistory.push(AppState::CHANGE_EVENT_LOAD);
                break;

            case AppState::ROOT_GEOMETRY_LOAD:
                this->nextState = AppState::SHOW_LOADING;
                this->message = "Loading EVENT:\n" + std::to_string(this->pendingNumber) + " - RUN N° " + std::to_string(this->run->runNumber);
                this->statesHistory.push(AppState::EVENT_LOAD);
                break;

            default:
                break;
        }
    }

    void AppStateManager::close() {
        this->nextState = AppState::CLOSED;
    }

    AppState AppStateManager::getCurrentState() {
        return this->currentState;
    }

    std::string AppStateManager::getMessage() {
        return this->message;
    }

    const RunData* AppStateManager::getRun() {
        return this->run.get();
    }

    const EventData* AppStateManager::getEvent() {
        return this->event.get();
    }

    void AppStateManager::numberSelected(int64_t number) {
        switch (this->currentState) {
            case AppState::RUN_CHOICE:
                this->pendingNumber = number;
                this->nextState = AppState::SHOW_LOADING;
                this->message = "Loading RUN:\n" + std::to_string(number);
                this->statesHistory.push(AppState::RUN_LOAD);
                break;

            case AppState::EVENT_CHOICE:
                this->pendingNumber = number;
                this->nextState = AppState::SHOW_LOADING;
                this->message = "Loading root geometry...";
                this->statesHistory.push(AppState::ROOT_GEOMETRY_LOAD);
                break;

            default:
                break;
        }
    }

    int64_t AppStateManager::getPendingNumber() {
        return this->pendingNumber;
    }

    void AppStateManager::runLoaded(RunData* runData) {
        switch (this->currentState) {
            case AppState::RUN_LOAD:
                this->run = std::unique_ptr<RunData>(runData);
                this->nextState = AppState::SHOW_LOADING;
                this->detectorPath = std::string(constants::paths::GEOMETRIES) + this->run->geoName + ".gltf"; 
                this->message = "Loading default geometry file:\n" + this->detectorPath;
                this->statesHistory.push(AppState::DEFAULT_GEOMETRY_LOAD);
                break;

            default:
                std::cerr << "ERROR! Loading run not allowed in state: " << appStateToString(this->currentState) << std::endl;
                break;
        }
    }

    void AppStateManager::eventLoaded(EventData* eventData) {
        switch (this->currentState) {
            case AppState::EVENT_LOAD:
            case AppState::CHANGE_EVENT_LOAD:
                this->event = std::unique_ptr<EventData>(eventData);
                this->nextState = AppState::TRACKBALL;
                break;

            default:
                std::cerr << "ERROR! Loading event not allowed in state: " << appStateToString(this->currentState) << std::endl;
                break;
        }
    }

    void AppStateManager::openGeometryDialog() {
        switch (this->currentState) {
            case AppState::DEFAULT_GEOMETRY_FAILED:
                this->nextState = AppState::USER_GEOMETRY_CHOICE;
                break;

            case AppState::TRACKBALL:
            case AppState::MOVING_TRACKBALL:
            case AppState::PAN:
            case AppState::MOVING_PAN:
                this->nextState = AppState::CHANGE_GEOMETRY_START;
                break;

            default:
                std::cerr << "ERROR! Opening geometry chooser not allowed in state: " << appStateToString(this->currentState) << std::endl;
                break;
        }
    }

    void AppStateManager::geometryFileSelected(std::string filePath) {
        switch (this->currentState) {
            case AppState::USER_GEOMETRY_CHOICE:
                this->statesHistory.push(AppState::USER_GEOMETRY_LOAD);
                break;

            case AppState::CHANGE_GEOMETRY_BROWSE:
                this->statesHistory.push(AppState::CHANGE_GEOMETRY_LOAD);
                break;

            default:
                return;
                break;
        }

        this->detectorPath = filePath;
        this->nextState = AppState::SHOW_LOADING;
        this->message = "Loading geometry file:\n" + filePath;
    }

    void AppStateManager::resetDefaultGeometry() {
        switch (this->currentState) {
            case AppState::TRACKBALL:
            case AppState::MOVING_TRACKBALL:
            case AppState::PAN:
            case AppState::MOVING_PAN:
                this->detectorPath = std::string(constants::paths::GEOMETRIES) + this->run->geoName + ".gltf"; 
                this->nextState = AppState::SHOW_LOADING;
                this->message = "Loading default geometry file:\n" + this->detectorPath;
                this->statesHistory.push(AppState::CHANGE_GEOMETRY_LOAD);
                break;

            default:
                break;
        }
    }

    std::string AppStateManager::getDetectorPath() {
        return this->detectorPath;
    }

    void AppStateManager::geometryLoaded() {
        switch (this->currentState) {
            case AppState::USER_GEOMETRY_LOAD:
            case AppState::DEFAULT_GEOMETRY_LOAD:
                this->nextState = AppState::EVENT_CHOICE;
                break;

            case AppState::CHANGE_GEOMETRY_LOAD:
                this->nextState = AppState::TRACKBALL;
                break;

            default:
                std::cerr << "ERROR! Loading geometry not allowed in state: " << appStateToString(this->currentState) << std::endl;
                break;
        }
    }

    void AppStateManager::errorInitializing(std::string exceptionMessage) {
        switch (this->currentState) {

            case AppState::RUN_LOAD:
                this->statesHistory.push(AppState::RUN_CHOICE);
                this->message = "Invalid run number chosen:\n" + std::to_string(this->pendingNumber);
                break;

            case AppState::EVENT_LOAD:
                this->statesHistory.push(AppState::EVENT_CHOICE);
                this->message = "Invalid event number chosen:\n" + std::to_string(this->pendingNumber) + " - RUN N° " + std::to_string(this->run->runNumber);
                break;

            case AppState::DEFAULT_GEOMETRY_LOAD:
                this->statesHistory.push(AppState::DEFAULT_GEOMETRY_FAILED);
                this->message = "Default geometry file not found:\n" + this->detectorPath;
                break;

            case AppState::USER_GEOMETRY_LOAD:
                this->statesHistory.push(AppState::DEFAULT_GEOMETRY_FAILED);
                this->message = "Error loading file:\n" + this->detectorPath;
                break;

            case AppState::CHANGE_GEOMETRY_LOAD:
                this->statesHistory.push(AppState::CHANGE_GEOMETRY_START);
                this->message = "Error loading file:\n" + this->detectorPath;
                break;

            case AppState::CHANGE_EVENT_LOAD:
                this->statesHistory.push(AppState::TRACKBALL);
                this->message = "Invalid event number chosen:\n" + std::to_string(this->pendingNumber) + " - RUN N° " + std::to_string(this->run->runNumber);
                break;

            default:
                return;
        }

        this->nextState = AppState::INIT_ERROR;

        if (!exceptionMessage.empty()) this->message += "\n\nException message:\n" + exceptionMessage;
    }

    void AppStateManager::previousStep() {
        switch (this->currentState) {
            case AppState::USER_GEOMETRY_CHOICE:
                this->nextState = AppState::DEFAULT_GEOMETRY_FAILED;
                break;

            case AppState::DEFAULT_GEOMETRY_FAILED:
                this->nextState = AppState::RUN_CHOICE;
                break;

            case AppState::EVENT_CHOICE:
                this->nextState = AppState::RUN_CHOICE;
                break;

            case AppState::INIT_ERROR:
                this->setNextStateFromHistory();
                break;

            case AppState::CHANGE_GEOMETRY_BROWSE:
                this->nextState = AppState::TRACKBALL;
                break;

            default:
                break;
        }
    }

    void AppStateManager::changeEvent(int64_t offset) {
        if (!isInteractionState(this->currentState)) {
            std::cerr << "ERROR! Changing event not allowed in state: " << appStateToString(this->currentState) << std::endl;
            return;
        }

        this->nextState = AppState::CHANGE_EVENT_START;
        this->pendingNumber = this->event->id + offset;
    }

    void AppStateManager::toggleMovingTrackball() {
        switch (this->currentState) {
            case AppState::TRACKBALL:
                this->nextState = AppState::MOVING_TRACKBALL;
                break;
            case AppState::MOVING_TRACKBALL:
                this->nextState = AppState::TRACKBALL;
                break;
            default:
                std::cerr << "ERROR! Toggle moving trackball not allowed in state: " << appStateToString(this->currentState) << std::endl;
                break;
        }
    }

    void AppStateManager::shiftPressed() {
        switch (this->currentState) {
            case AppState::TRACKBALL:
                this->nextState = AppState::PAN;
                break;

            case AppState::MOVING_TRACKBALL:
                this->nextState = AppState::MOVING_PAN;
                break;

            default:
                break;
        }
    }

    void AppStateManager::shiftReleased() {
        switch (this->currentState) {
            case AppState::PAN:
                this->nextState = AppState::TRACKBALL;
                break;

            case AppState::MOVING_PAN:
                this->nextState = AppState::MOVING_TRACKBALL;
                break;

            default:
                break;
        }
    }

    void AppStateManager::toggleMovingPan() {
        switch (this->currentState) {
            case AppState::PAN:
                this->nextState = AppState::MOVING_PAN;
                break;
            case AppState::MOVING_PAN:
                this->nextState = AppState::PAN;
                break;
            default:
                std::cerr << "ERROR! Toggle moving pan not allowed in state: " << appStateToString(this->currentState) << std::endl;
                break;
        }
    }

    void AppStateManager::toggleImageExport() {
        switch (this->currentState) {
            case AppState::TRACKBALL:
            case AppState::MOVING_TRACKBALL:
            case AppState::PAN:
            case AppState::MOVING_PAN:
                this->nextState = AppState::EXPORT_IMAGE;
                break;

            case AppState::EXPORT_IMAGE:
                this->nextState = AppState::TRACKBALL;
                break;

            default:
                break;
        }
    }

    void AppStateManager::resetInteraction() {
        switch (this->currentState) {
            case AppState::TRACKBALL:
            case AppState::MOVING_TRACKBALL:
            case AppState::PAN:
            case AppState::MOVING_PAN:
                this->nextState = AppState::TRACKBALL;
                break;

            default:
                break;
        }
    }

    void AppStateManager::setNextStateFromHistory() {
        this->nextState = this->statesHistory.top();
        this->statesHistory.pop();
    }
}

#include "ui/Gui.hpp"

#include <iostream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuiFileDialog.h>
#include <glm/gtc/type_ptr.hpp>

#include "ui/JetBrainsMono.h"
#include "core/App.hpp"
#include "core/Constants.hpp"
#include "scene/Node.hpp"
#include "core/state/AppState.hpp"
#include "scene/Object.hpp"
#include "scene/colors/Modes.hpp"

namespace snd3D {

    Gui::Gui(App& _app, float _fontSize) : app(_app) {
        this->fontSize = _fontSize;

        // Init ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDoubleClickTime = 1; // Consider double click if it happens within 1 second
        io.FontGlobalScale = this->fontSize;
        io.IniFilename = NULL; // Deactivate ini file
        ImFont* font = io.Fonts->AddFontFromMemoryCompressedTTF( // Load JetBrainsMono
            jetbrains_mono_compressed_data,
            jetbrains_mono_compressed_size,
            constants::sizes::FONT
        );
        if (font == nullptr) {
            io.Fonts->AddFontDefault();
        }
        ImGui::StyleColorsDark(); // Set ImGUI dark theme
        ImGui_ImplGlfw_InitForOpenGL(this->app.windowManager->getWindow(), true);
        ImGui_ImplOpenGL3_Init("#version 330 core");

        try {
            this->logo = std::make_unique<Texture>(std::string("SND Logo"), std::string(constants::paths::SND_LOGO));
        }
        catch (const std::runtime_error& e) {
            std::cerr << "Exception caught when loading logo image:\n\t" << e.what() << std::endl;
        }

        this->clusterConfig = std::make_unique<ClusterConfiguration>(
            constants::defaults::clusters::VETO_MAX_GAP,
            constants::defaults::clusters::VETO_MIN_HIT,
            constants::defaults::clusters::SCIFI_MAX_GAP,
            constants::defaults::clusters::SCIFI_MIN_HIT,
            constants::defaults::clusters::US_MAX_GAP,
            constants::defaults::clusters::US_MIN_HIT,
            constants::defaults::clusters::DS_MAX_GAP,
            constants::defaults::clusters::DS_MIN_HIT
        );
    }

    Gui::~Gui() {
        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void Gui::update() {
        // Start ImGui Frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (this->app.stateManager.getCurrentState() != AppState::EXPORT_IMAGE) this->drawMenuBar();

        switch (this->app.stateManager.getCurrentState()) {

            case AppState::RUN_CHOICE:
            case AppState::CHANGE_RUN_CHOICE:
                this->drawRunDialog();
                break;

            case AppState::SHOW_LOADING:
            case AppState::RUN_LOAD:
            case AppState::EVENT_LOAD:
            case AppState::ROOT_GEOMETRY_LOAD:
            case AppState::USER_GEOMETRY_LOAD:
            case AppState::DEFAULT_GEOMETRY_LOAD:
            case AppState::CHANGE_EVENT_LOAD:
            case AppState::CHANGE_RUN_LOAD:
                this->drawLoadingData();
                this->needsFocus = true; // Reset the focus when something is loaded
                break;

            case AppState::EVENT_CHOICE:
            case AppState::CHANGE_EVENT_CHOICE:
                this->drawEventDialog();
                break;

            case AppState::DEFAULT_GEOMETRY_FAILED:
                this->drawDefaultGeometryFailed();
                break;

            case AppState::USER_GEOMETRY_CHOICE:
                this->drawGeometryFileDialog();
                break;

            case AppState::INIT_ERROR:
                this->drawInitializationError();
                break;

            case AppState::EXPORT_IMAGE:
                this->drawEventDetails();
                this->drawColorScale();
                this->drawClusterConfiguration();
                break;

            case AppState::INTERACTION:
                this->drawInspector();
                this->drawRenderOptions();
                this->drawEventDetails();
                this->drawColorScale();
                this->drawClusterConfiguration();
                break;

            case AppState::CHANGE_GEOMETRY_START: {
                this->drawLoadingData();
                IGFD::FileDialogConfig config;
                config.path = ".";
                ImGuiFileDialog::Instance()->OpenDialog("ChooseGeometryFile", "CHOOSE GEOMETRY FILE", ".gltf", config);
                } break;

            case AppState::CHANGE_GEOMETRY_BROWSE:
                this->drawGeometryFileDialog();
                break;

            default:
                break;
        }
    }

    void Gui::render() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void Gui::changeFontSize(float factor) {
        if (this->fontSize * factor < constants::limits::GUI_FONT_SIZE_MIN) return;
        this->fontSize *= factor;
        ImGuiIO& io = ImGui::GetIO();
        io.FontGlobalScale = this->fontSize;
    }

    bool Gui::isPointerUsedByGui() {
        ImGuiIO& io = ImGui::GetIO();
        return io.WantCaptureMouse;
    }

    void Gui::drawMenuBar() {
        bool interactionState = this->app.stateManager.getCurrentState() == AppState::INTERACTION;
        if (ImGui::BeginMainMenuBar()) {
            this->menuBarHeight = ImGui::GetWindowSize().y;
            if (ImGui::BeginMenu("File")) {
                if (!interactionState) ImGui::BeginDisabled();
                if (ImGui::MenuItem("Load New Run", "Ctrl + N")) {
                    this->app.stateManager.startRunChange();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Change Event", "Ctrl + E")) {
                    this->app.stateManager.startEventChange();
                }
                if (ImGui::MenuItem("Next Event", "K")) {
                    this->app.stateManager.changeEvent(1);
                }
                if (ImGui::MenuItem("Previous Event", "J")) {
                    this->app.stateManager.changeEvent(-1);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Change Geometry", "Ctrl + G")) {
                    this->app.stateManager.openGeometryDialog();
                }
                if (ImGui::MenuItem("Load Default Geometry")) {
                    this->app.stateManager.resetDefaultGeometry();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Save Image", "Ctrl + P")) {
                    this->app.stateManager.toggleImageExport();
                }
                if (!interactionState) ImGui::EndDisabled();
                ImGui::Separator();
                if (ImGui::MenuItem("Quit", "Alt + F4")) {
                    this->app.stateManager.close();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                if (!interactionState) ImGui::BeginDisabled();
                bool lighting = this->app.settings.isLightingEnabled();
                if (ImGui::MenuItem("Lighting", "L", lighting)) {
                    this->app.settings.toggleLighting();
                }
                bool ortographic = this->app.scene->viewport->isOrthographic();
                if (ImGui::MenuItem("Ortographic projection", "P", ortographic)) {
                    this->app.scene->viewport->toggleProjectionType();
                }
                if (ImGui::BeginMenu("Standard views")) {
                    if (ImGui::MenuItem("Top (XZ)", "7"))   this->app.scene->viewport->setDirection(Camera::Directions::ALIGN_Y);
                    if (ImGui::MenuItem("Front (XY)", "1")) this->app.scene->viewport->setDirection(Camera::Directions::ALIGN_Z);
                    if (ImGui::MenuItem("Right (YZ)", "3")) this->app.scene->viewport->setDirection(Camera::Directions::ALIGN_X);
                    ImGui::Separator();
                    if (ImGui::MenuItem("Bottom (-XZ)", "Ctrl + 7"))  this->app.scene->viewport->setDirection(Camera::Directions::ALIGN_Y_NEG);
                    if (ImGui::MenuItem("Back (-XY)", "Ctrl + 1"))    this->app.scene->viewport->setDirection(Camera::Directions::ALIGN_Z_NEG);
                    if (ImGui::MenuItem("Left (-YZ)", "Ctrl + 3"))    this->app.scene->viewport->setDirection(Camera::Directions::ALIGN_X_NEG);
                    ImGui::Separator();
                    if (ImGui::MenuItem("Isometric 1", "0"))  this->app.scene->viewport->setDirection(Camera::Directions::ISOMETRIC1);
                    if (ImGui::MenuItem("Isometric 2", "Ctrl + 0"))  this->app.scene->viewport->setDirection(Camera::Directions::ISOMETRIC2);
                    ImGui::EndMenu();
                }
                ImGui::Separator();
                bool inspector = this->app.settings.isSceneInspectorActive();
                if (ImGui::MenuItem("Scene inspector", "S", inspector)) {
                    this->app.settings.toggleSceneInspector();
                }
                bool renderOptions = this->app.settings.isRenderOptionsActive();
                if (ImGui::MenuItem("Render options", "G", renderOptions)) {
                    this->app.settings.toggleRenderOptions();
                }
                bool eventInfo = this->app.settings.isEventInfoActive();
                if (ImGui::MenuItem("Event Info", "I", eventInfo)) {
                    this->app.settings.toggleEventInfo();
                }
                bool clusterConfigInfo = this->app.settings.isClusterConfigInfoActive();
                if (ImGui::MenuItem("Cluster Configuration", nullptr, clusterConfigInfo)) {
                    this->app.settings.toggleClusterConfigInfo();
                }
                bool colorScale = this->app.settings.isColorScaleActive();
                if (ImGui::MenuItem("Color Scale", "C", colorScale)) {
                    this->app.settings.toggleColorScale();
                }
                if (!interactionState) ImGui::EndDisabled();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Display")) {
                if (!interactionState) ImGui::BeginDisabled();
                bool pivot = this->app.settings.isCameraPivotActive();
                if (ImGui::MenuItem("Show Pivot", "X", pivot)) {
                    this->app.settings.toggleCameraPivot();
                }
                bool axis = this->app.settings.isAxisWidgetActive();
                if (ImGui::MenuItem("Show Axis", "A", axis)) {
                    this->app.settings.toggleAxisWidget();
                }
                if (!interactionState) ImGui::EndDisabled();
                ImGui::Separator();
                if (ImGui::MenuItem("Font +", "Ctrl + +")) {
                    this->changeFontSize(constants::factors::GUI_FONT_RESIZE);
                }
                if (ImGui::MenuItem("Font -", "Ctrl + -")) {
                    this->changeFontSize(1 / constants::factors::GUI_FONT_RESIZE);
                }
                bool vsyncEnabled = this->app.windowManager->isVsyncActive();
                if (ImGui::MenuItem("V-Sync", "V", vsyncEnabled)) {
                    this->app.windowManager->toggleVsync();
                }
                bool fullScreen = this->app.windowManager->isFullScreen();
                if (ImGui::MenuItem("Full Screen", "F11", fullScreen)) {
                    this->app.windowManager->toggleFullScreen();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Camera")) {
                if (!interactionState) ImGui::BeginDisabled();
                if (ImGui::MenuItem("Zoom In", "Scroll Up")) {
                    this->app.scene->viewport->zoom(constants::factors::ZOOM);
                }
                if (ImGui::MenuItem("Zoom Out", "Scroll Down")) {
                    this->app.scene->viewport->zoom(-constants::factors::ZOOM);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Rotate Up", "Up")) {
                    this->app.scene->viewport->rotateByAngles(0, constants::factors::ROTATION_SPEED);
                }
                if (ImGui::MenuItem("Rotate Down", "Down")) {
                    this->app.scene->viewport->rotateByAngles(0, -constants::factors::ROTATION_SPEED);
                }
                if (ImGui::MenuItem("Rotate Right", "Right")) {
                    this->app.scene->viewport->rotateByAngles(constants::factors::ROTATION_SPEED, 0);
                }
                if (ImGui::MenuItem("Rotate Left", "Left")) {
                    this->app.scene->viewport->rotateByAngles(-constants::factors::ROTATION_SPEED, 0);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Move Forward", "Shift + Scroll Up")) {
                    this->app.scene->viewport->movePerpendicular(0, constants::factors::PAN);
                }
                if (ImGui::MenuItem("Move Backward", "Shift + Scroll Down")) {
                    this->app.scene->viewport->movePerpendicular(0, -constants::factors::PAN);
                }
                if (ImGui::MenuItem("Move Up", "Shift + Up")) {
                    this->app.scene->viewport->moveParallel(0, constants::factors::PAN);
                }
                if (ImGui::MenuItem("Move Down", "Shift + Down")) {
                    this->app.scene->viewport->moveParallel(0, -constants::factors::PAN);
                }
                if (ImGui::MenuItem("Move Right", "Shift + Right")) {
                    this->app.scene->viewport->moveParallel(constants::factors::PAN, 0);
                }
                if (ImGui::MenuItem("Move Left", "Shift + Left")) {
                    this->app.scene->viewport->moveParallel(-constants::factors::PAN, 0);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Reset Position", "R")) {
                    this->app.scene->viewport->reset();
                }
                if (!interactionState) ImGui::EndDisabled();
                ImGui::EndMenu();
            }

            // Move FPS label at the end of the window
            float fps = ImGui::GetIO().Framerate;
            char label[50];
            sprintf(label, "%s - %05.1f FPS", appStateToString(this->app.stateManager.getCurrentState()), fps);
            float textWidth = ImGui::CalcTextSize(label).x;
            float padding = ImGui::GetStyle().ItemSpacing.x;
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - textWidth - padding);
            ImGui::TextDisabled("%s", label);

            ImGui::EndMainMenuBar();
        }
    }

    void Gui::drawInspector() {
        bool open = this->app.settings.isSceneInspectorActive();

        if (open) {
            ImGui::SetNextWindowSizeConstraints(
                ImVec2(0.0f, 0.0f), // No min size
                ImVec2(
                    this->app.windowManager->getCurrentResolution().x - constants::sizes::PADDING * 2,
                    this->app.windowManager->getCurrentResolution().y - this->menuBarHeight - constants::sizes::PADDING * 2
                )
            );

            ImGui::SetNextWindowPos(ImVec2(constants::sizes::PADDING, this->menuBarHeight + constants::sizes::PADDING), ImGuiCond_Always);

            ImGui::SetNextWindowSize(ImVec2(400, this->app.windowManager->getCurrentResolution().y - this->menuBarHeight - constants::sizes::PADDING * 2), ImGuiCond_Once);

            ImGui::Begin("Scene", &open, ImGuiWindowFlags_NoMove);

            this->drawObjectTree("SND", this->app.scene->detector.get());

            this->drawObjectTree("HITS", this->app.scene->hits.get());

            ImGui::End();
        }

        if (open != this->app.settings.isSceneInspectorActive()) this->app.settings.toggleSceneInspector();
    }

    void Gui::drawRenderOptions() {
        bool open = this->app.settings.isRenderOptionsActive();

        if (open) {
            ImGui::SetNextWindowSizeConstraints(
                ImVec2(0.0f, 0.0f), // No min size
                ImVec2(
                    this->app.windowManager->getCurrentResolution().x - constants::sizes::PADDING * 2,
                    this->app.windowManager->getCurrentResolution().y - this->menuBarHeight - constants::sizes::PADDING * 2
                )
            );

            ImGui::SetNextWindowPos(ImVec2(
                this->app.windowManager->getCurrentResolution().x - constants::sizes::PADDING,
                this->menuBarHeight + constants::sizes::PADDING),
                ImGuiCond_Always,
                ImVec2(1.0f, 0.0f) // Ser right pivot
            );

            ImGui::SetNextWindowSize(ImVec2(450, 0), ImGuiCond_Once);

            ImGui::Begin("Render Options", &open, ImGuiWindowFlags_NoMove);

            ImGui::Text("Projection Mode:");
            int projMode = this->app.scene->viewport->isOrthographic() ? 1 : 0;

            if (ImGui::RadioButton("Perspective", &projMode, 0)) {
                if (this->app.scene->viewport->isOrthographic()) {
                    this->app.scene->viewport->toggleProjectionType();
                }
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Orthographic", &projMode, 1)) {
                if (!this->app.scene->viewport->isOrthographic()) {
                    this->app.scene->viewport->toggleProjectionType();
                }
            }

            ImGui::NewLine();
            ImGui::Separator();
            ImGui::NewLine();

            bool lighting = this->app.settings.isLightingEnabled();
            if (ImGui::Checkbox("Lighting", &lighting)) {
                this->app.settings.toggleLighting();
            }
            ImGui::NewLine();

            bool transparency = this->app.settings.isTransparencyEnabled();
            if (ImGui::Checkbox("Transparency", &transparency)) {
                this->app.settings.toggleTransparency();
            }
            if (transparency) {
                ImGui::NewLine();
                float edgeAlphaValue = this->app.settings.getEdgeAlphaValue();
                if (ImGui::SliderFloat(" Edge Alpha", &edgeAlphaValue, constants::limits::ALPHA_VALUE_MIN, constants::limits::ALPHA_VALUE_MAX)) {
                    this->app.settings.setEdgeAlphaValue(edgeAlphaValue);
                }
                float faceAlphaValue = this->app.settings.getFaceAlphaValue();
                if (ImGui::SliderFloat(" Face Alpha", &faceAlphaValue, constants::limits::ALPHA_VALUE_MIN, constants::limits::ALPHA_VALUE_MAX)) {
                    this->app.settings.setFaceAlphaValue(faceAlphaValue);
                }
                float edgeThickness = this->app.settings.getEdgeThickness();
                if (ImGui::SliderFloat(" Edge Weight", &edgeThickness, constants::limits::EDGE_THICKNESS_MIN, constants::limits::EDGE_THICKNESS_MAX)) {
                    this->app.settings.setEdgeThickness(edgeThickness);
                }
                ImGui::NewLine();
                if (ImGui::Button("Reset")) {
                    this->app.settings.setEdgeAlphaValue(constants::defaults::EDGE_ALPHA_VALUE);
                    this->app.settings.setFaceAlphaValue(constants::defaults::FACE_ALPHA_VALUE);
                    this->app.settings.setEdgeThickness(constants::defaults::EDGE_THICKNESS);
                }
                ImGui::NewLine();
            }

            ImGui::NewLine();
            ImGui::Separator();
            ImGui::NewLine();

            glm::vec3 bgColor = this->app.settings.getBackgroundColor();
            if (ImGui::ColorEdit3("Background Color", glm::value_ptr(bgColor))) {
                this->app.settings.setBackgroundColor(bgColor);
            }
            ImGui::End();
        }

        if (open != this->app.settings.isRenderOptionsActive()) this->app.settings.toggleRenderOptions();
    }

    void Gui::drawObjectTree(const std::string& label, Object* obj) {
        if (!obj || !obj->rootNode) return;

        ImGui::PushID(obj); // Create unique ID to identify the object into the gui

        bool active = obj->active;
        if (ImGui::Checkbox("##objActive", &active)) {
            obj->setGlobalActive(active);
        }
        ImGui::SameLine();

        if (ImGui::CollapsingHeader(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {

            float height = ImGui::GetContentRegionAvail().y * 0.4f;
            ImGui::BeginChild((label + "Tree").c_str(), ImVec2(0, height), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeY, ImGuiWindowFlags_HorizontalScrollbar);

            if (ImGui::Button("Show All")) {
                obj->setGlobalActive(true);
            }
            ImGui::SameLine();
            if (ImGui::Button("Hide All")) {
                obj->setGlobalActive(false);
            }

            this->drawNodeTree(obj->rootNode.get(), obj->active); // Start showing children recursively

            ImGui::EndChild();
        }

        ImGui::PopID(); // End using this object ID
    }

    void Gui::drawNodeTree(Node* node, bool parentActive) {
        if (!node) return;

        ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

        if (node->childrenNode.empty() && node->meshes.empty()) {
            nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        }

        ImGui::PushID(node); // Create unique ID to identify the node into the gui

        bool isCurrentDisabled = !parentActive;
        ImGui::BeginDisabled(isCurrentDisabled);
        
        bool active = node->active;
        if (ImGui::Checkbox("##nodeActive", &active)) {
            node->setGlobalActive(active);
        }
        ImGui::SameLine();

        bool isOpened = ImGui::TreeNodeEx((void*)node, nodeFlags, "[Node] - %s", node->name.c_str()); // Create the tree for this node

        if (isOpened) {
            bool childrenActive = parentActive && node->active;

            for (auto& mesh : node->meshes) {
                ImGui::PushID(mesh.get()); // Create unique ID to identify the mesh into the gui

                ImGuiTreeNodeFlags meshFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;

                ImGui::BeginDisabled(!childrenActive);

                ImGui::Checkbox("##meshActive", &mesh->active);
                ImGui::SameLine();

                ImGui::TreeNodeEx((void*)mesh.get(), meshFlags, "[Mesh] - %s", mesh->name.c_str());

                ImGui::EndDisabled();
                ImGui::PopID();
            }

            for (auto& child : node->childrenNode) {
                drawNodeTree(child.get(), childrenActive);
            }

            // If the node isn't a leaf, close the TreeNodeEx
            if (!(nodeFlags & ImGuiTreeNodeFlags_Leaf)) {
                ImGui::TreePop();
            }
        }

        ImGui::EndDisabled();
        ImGui::PopID();
    }

    void Gui::drawGeometryFileDialog() {
        ImGui::SetNextWindowPos(ImVec2(constants::sizes::PADDING, this->menuBarHeight + constants::sizes::PADDING), ImGuiCond_Always);

        ImGui::SetNextWindowSize(ImVec2(
            this->app.windowManager->getCurrentResolution().x - constants::sizes::PADDING * 2,
            this->app.windowManager->getCurrentResolution().y - this->menuBarHeight - constants::sizes::PADDING * 2),
            ImGuiCond_Once
        );

        if (ImGuiFileDialog::Instance()->Display(
            "ChooseGeometryFile",
            ImGuiWindowFlags_NoCollapse,
            ImVec2(0.0f, 0.0f),
            ImVec2(
                this->app.windowManager->getCurrentResolution().x - constants::sizes::PADDING * 2,
                this->app.windowManager->getCurrentResolution().y - this->menuBarHeight - constants::sizes::PADDING * 2
            )
        )) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

                this->app.stateManager.geometryFileSelected(filePathName);
            } else {
                this->app.stateManager.previousStep();
            }

            ImGuiFileDialog::Instance()->Close();
        }
    }

    void Gui::drawRunDialog() {
        ImGui::SetNextWindowPos(ImVec2(constants::sizes::PADDING, this->menuBarHeight + constants::sizes::PADDING), ImGuiCond_Always);

        ImGui::SetNextWindowSize(ImVec2(
            this->app.windowManager->getCurrentResolution().x - constants::sizes::PADDING * 2,
            this->app.windowManager->getCurrentResolution().y - this->menuBarHeight - constants::sizes::PADDING * 2),
            ImGuiCond_Always
        );

        ImGui::Begin("SELECT RUN", NULL,
            ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_AlwaysAutoResize
            | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoCollapse
        );
        ImGui::TextWrapped("Insert the RUN number");
        ImGui::NewLine();
        if (this->needsFocus) {
            ImGui::SetKeyboardFocusHere(0);
            this->needsFocus = false;
        }
        ImGui::InputScalar("Run Number", ImGuiDataType_S64, &this->runInputNumber);
        ImGui::NewLine();

        bool isInvalid = this->runInputNumber < 0;

        if (isInvalid) ImGui::BeginDisabled();
        if ((ImGui::Button("Continue") || ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) && !isInvalid) {
            this->app.stateManager.runNumberSelected(this->runInputNumber);
        }
        if (isInvalid) ImGui::EndDisabled();

        ImGui::NewLine();
        ImGui::Separator();
        ImGui::NewLine();
        if (ImGui::Button("Go Back")) {
            this->app.stateManager.previousStep();
            this->needsFocus = true; // Reset the focus
        }
        ImGui::SameLine();
        if (ImGui::Button("Quit")) {
            this->app.stateManager.close();
        }

        ImGui::End();
    }

    void Gui::drawLoadingData() {
        ImGui::SetNextWindowPos(ImVec2(constants::sizes::PADDING, this->menuBarHeight + constants::sizes::PADDING), ImGuiCond_Always);

        ImGui::SetNextWindowSize(ImVec2(
            this->app.windowManager->getCurrentResolution().x - constants::sizes::PADDING * 2,
            this->app.windowManager->getCurrentResolution().y - this->menuBarHeight - constants::sizes::PADDING * 2),
            ImGuiCond_Always
        );

        ImGui::Begin("LOADING DATA", NULL,
            ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_AlwaysAutoResize
            | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoCollapse
        );
        ImGui::TextWrapped("Please Wait...");
        ImGui::NewLine();
        ImGui::TextWrapped(this->app.stateManager.getMessage().c_str());

        ImGui::End();
    }

    void Gui::drawEventDialog() {
        ImGui::SetNextWindowPos(ImVec2(constants::sizes::PADDING, this->menuBarHeight + constants::sizes::PADDING), ImGuiCond_Always);

        ImGui::SetNextWindowSize(ImVec2(
            this->app.windowManager->getCurrentResolution().x - constants::sizes::PADDING * 2,
            this->app.windowManager->getCurrentResolution().y - this->menuBarHeight - constants::sizes::PADDING * 2),
            ImGuiCond_Always
        );

        ImGui::Begin("SELECT EVENT", NULL,
            ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_AlwaysAutoResize
            | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoCollapse
        );

        const RunData* run = this->app.stateManager.getRun();
        ImGui::Text("RUN N° %d", run->runNumber);
        ImGui::Text("Date: %s", run->startDate.c_str());
        ImGui::Text("Num entries: %d", run->totalEvents);
        ImGui::NewLine();
        ImGui::TextWrapped("Insert the EVENT number");
        ImGui::NewLine();
        if (this->needsFocus) {
            ImGui::SetKeyboardFocusHere(0);
            this->needsFocus = false;
        }
        ImGui::InputScalar("Event Number", ImGuiDataType_S64, &this->eventInputNumber);
        ImGui::NewLine();


        ImGui::BeginTable("EventData", 2, ImGuiTableFlags_None);

        ImGui::TableSetupColumn("ClusterConfig", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("ColorConfig", ImGuiTableColumnFlags_WidthFixed);

        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);

        if (ImGui::CollapsingHeader("Cluster Configuration", ImGuiTreeNodeFlags_DefaultOpen)) {
            // Change tab colors
            ImGui::PushStyleColor(ImGuiCol_Tab,          ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_TabHovered,   ImVec4(0.25f, 0.40f, 0.65f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_TabActive,    ImVec4(0.20f, 0.50f, 0.85f, 1.0f));

            if (ImGui::BeginTabBar("Cluster Configuration")) {
                if (ImGui::BeginTabItem("Veto")) {
                    ImGui::InputInt("Min Entries", &this->clusterConfig->vetoMinHitInCluster);
                    ImGui::InputDouble("Max Gap", &this->clusterConfig->vetoMaxGap, 0, 0, "%.2f");
                    ImGui::EndTabItem();
                }
                
                if (ImGui::BeginTabItem("SciFi")) {
                    ImGui::InputInt("Min Entries", &this->clusterConfig->sciFiMinHitInCluster);
                    ImGui::InputDouble("Max Gap", &this->clusterConfig->sciFiMaxGap, 0, 0, "%.2f");
                    ImGui::EndTabItem();
                }
                
                if (ImGui::BeginTabItem("US")) {
                    ImGui::InputInt("Min Entries", &this->clusterConfig->usMinHitInCluster);
                    ImGui::InputDouble("Max Gap", &this->clusterConfig->usMaxGap, 0, 0, "%.2f");
                    ImGui::EndTabItem();
                }
                
                if (ImGui::BeginTabItem("DS")) {
                    ImGui::InputInt("Min Entries", &this->clusterConfig->dsMinHitInCluster);
                    ImGui::InputDouble("Max Gap", &this->clusterConfig->dsMaxGap, 0, 0, "%.2f");
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
            ImGui::PopStyleColor(3);
        }

        ImGui::TableSetColumnIndex(1);

        ImGui::Indent(50.0f);

        if (ImGui::CollapsingHeader("Color Configuration", ImGuiTreeNodeFlags_DefaultOpen)) {

            int currentVariable = (int)this->app.settings.getColorVariable();

            ImGui::Text("Color by:");
            if (ImGui::RadioButton("Energy", &currentVariable, (int)ColorVariable::ENERGY)) {
                this->app.settings.setColorVariable(ColorVariable::ENERGY);
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Time", &currentVariable, (int)ColorVariable::TIME)) {
                this->app.settings.setColorVariable(ColorVariable::TIME);
            }

            int currentMode = (int)this->app.settings.getColorScalingMode();

            ImGui::Text("Scale:");
            if (ImGui::RadioButton("Linear", &currentMode, (int)ColorScalingMode::LINEAR)) {
                this->app.settings.setColorScalingMode(ColorScalingMode::LINEAR);
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Logarithmic", &currentMode, (int)ColorScalingMode::LOGARITHMIC)) {
                this->app.settings.setColorScalingMode(ColorScalingMode::LOGARITHMIC);
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Monochrome", &currentMode, (int)ColorScalingMode::FIXED)) {
                this->app.settings.setColorScalingMode(ColorScalingMode::FIXED);
            }

        }

        ImGui::Unindent(50.0f);

        ImGui::EndTable();

        ImGui::NewLine();

        bool isInvalid = this->eventInputNumber < 0 || this->eventInputNumber >= run->totalEvents;

        if (isInvalid) ImGui::BeginDisabled();
        if ((ImGui::Button("Continue") || ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) && !isInvalid) {
            this->app.stateManager.eventNumberSelected(this->eventInputNumber, new ClusterConfiguration(*this->clusterConfig));
        }
        if (isInvalid) ImGui::EndDisabled();

        ImGui::NewLine();
        ImGui::Separator();
        ImGui::NewLine();
        if (ImGui::Button("Go Back")) {
            this->app.stateManager.previousStep();
            this->needsFocus = true; // Reset the focus
        }
        ImGui::SameLine();
        if (ImGui::Button("Quit")) {
            this->app.stateManager.close();
        }

        ImGui::End();
    }

    void Gui::drawDefaultGeometryFailed() {
        ImGui::SetNextWindowPos(ImVec2(constants::sizes::PADDING, this->menuBarHeight + constants::sizes::PADDING), ImGuiCond_Always);

        ImGui::SetNextWindowSize(ImVec2(
            this->app.windowManager->getCurrentResolution().x - constants::sizes::PADDING * 2,
            this->app.windowManager->getCurrentResolution().y - this->menuBarHeight - constants::sizes::PADDING * 2),
            ImGuiCond_Always
        );

        ImGui::Begin("SELECT GEOMETRY", NULL,
            ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_AlwaysAutoResize
            | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoCollapse
        );

        const RunData* run = this->app.stateManager.getRun();
        ImGui::Text("RUN N° %d", run->runNumber);
        ImGui::Text("Start date: %s", run->startDate.c_str());
        ImGui::Text("Required geometry file: %s", run->geoName.c_str());
        ImGui::NewLine();
        ImGui::TextWrapped("I wasn't able to open the file:");
        ImGui::TextWrapped(this->app.stateManager.getDetectorPath().c_str());
        ImGui::NewLine();
        ImGui::TextWrapped("Open manually a geometry asset to initialize the 3D viewport.");
        ImGui::TextWrapped("Click the button below to browse your local files.");

        ImGui::NewLine();
        if (ImGui::Button("Browse")) {
            this->app.stateManager.openGeometryDialog();
            IGFD::FileDialogConfig config;
            config.path = ".";
            ImGuiFileDialog::Instance()->OpenDialog("ChooseGeometryFile", "CHOOSE GEOMETRY FILE", ".gltf", config);
        }
        ImGui::NewLine();
        ImGui::Separator();
        ImGui::NewLine();
        if (ImGui::Button("Go Back")) {
            this->app.stateManager.previousStep();
            this->needsFocus = true; // Reset the focus
        }
        ImGui::SameLine();
        if (ImGui::Button("Quit")) {
            this->app.stateManager.close();
        }

        ImGui::End();
    }

    void Gui::drawInitializationError() {
        ImGui::SetNextWindowPos(ImVec2(constants::sizes::PADDING, this->menuBarHeight + constants::sizes::PADDING), ImGuiCond_Always);

        ImGui::SetNextWindowSize(ImVec2(
            this->app.windowManager->getCurrentResolution().x - constants::sizes::PADDING * 2,
            this->app.windowManager->getCurrentResolution().y - this->menuBarHeight - constants::sizes::PADDING * 2),
            ImGuiCond_Always
        );

        ImGui::Begin("INITIALIZATION ERROR", NULL,
            ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_AlwaysAutoResize
            | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoCollapse
        );

        ImGui::TextWrapped(this->app.stateManager.getMessage().c_str());
        ImGui::NewLine();
        ImGui::Separator();
        ImGui::NewLine();
        if (ImGui::Button("Retry") || ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
            this->app.stateManager.previousStep();
        }
        ImGui::SameLine();
        if (ImGui::Button("Quit")) {
            this->app.stateManager.close();
        }

        ImGui::End();
    }

    void Gui::drawEventDetails() {
        bool open = this->app.settings.isEventInfoActive();

        if (open) {

            ImGui::SetNextWindowPos(ImVec2(
                constants::sizes::PADDING,
                this->app.windowManager->getCurrentResolution().y - constants::sizes::PADDING),
                ImGuiCond_Always,
                ImVec2(0.0f, 1.0f) // Set bottom-left pivot
            );

            ImGuiWindowFlags windowFlags = 
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_AlwaysAutoResize;

            if (this->app.stateManager.getCurrentState() == AppState::EXPORT_IMAGE) windowFlags |= ImGuiWindowFlags_NoTitleBar; // In the screenshot the title bar must disappear


            ImGui::Begin(
                "Event Info",
                &open,
                windowFlags
            );
            ImGui::BeginTable("EventInfo", 3, ImGuiTableFlags_None);

            ImGui::TableSetupColumn("Logo", ImGuiTableColumnFlags_WidthFixed, this->logo.get() != nullptr ? this->logo->getWidth() : 128);
            ImGui::TableSetupColumn("Labels", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Values", ImGuiTableColumnFlags_WidthFixed);

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            if (this->logo.get() != nullptr) {
                ImGui::Image(this->logo->getProgramId(), ImVec2(this->logo->getWidth(), this->logo->getHeight()));
            } else {
                ImGui::Button("SND LOGO\nNot Found", ImVec2(128, 128));
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::Indent(10.0f);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            ImGui::Text("Run N°");
            ImGui::Spacing();
            ImGui::Text("Event N°");
            ImGui::Spacing();
            ImGui::Text("Time (GMT)");
            ImGui::PopStyleColor();
            ImGui::Unindent(10.0f);

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%d", this->app.stateManager.getRun()->runNumber);
            ImGui::Spacing();
            ImGui::Text("%d", this->app.stateManager.getEvent()->getId());
            ImGui::Spacing();
            ImGui::Text(this->app.stateManager.getEvent()->getDateTime().c_str());

            ImGui::EndTable();
            ImGui::End();
        }

        if (open != this->app.settings.isEventInfoActive()) this->app.settings.toggleEventInfo();
    }

    void Gui::drawColorScale() {
        bool open = this->app.settings.isColorScaleActive();

        if (open) {

            ImGui::SetNextWindowPos(ImVec2(
                this->app.windowManager->getCurrentResolution().x * 2 / 3,
                this->app.windowManager->getCurrentResolution().y - constants::sizes::PADDING),
                ImGuiCond_FirstUseEver,
                ImVec2(0.5f, 1.0f) // Set low - center pivot
            );

            ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;

            if (this->app.stateManager.getCurrentState() == AppState::EXPORT_IMAGE) windowFlags |= ImGuiWindowFlags_NoTitleBar; // In the screenshot the title bar must disappear

            ImGui::Begin("Color Scale", &open, windowFlags);

            if (this->app.scene->colorPalette.get() != nullptr) {
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                ImVec2 cursorPos = ImGui::GetCursorScreenPos();

                const auto& colors = this->app.scene->colorPalette->getScale();
                const auto& range = this->app.scene->colorPalette->getRange();

                int numSegments = colors.size() - 1;
                float segmentWidth = constants::sizes::COLOR_SCALE_WIDTH / numSegments;

                for (int i = 0; i < numSegments; ++i) {
                    // Start and end point of each segment (left bottom and right top corners of the rectangle)
                    ImVec2 start = ImVec2(cursorPos.x + (i * segmentWidth), cursorPos.y);
                    ImVec2 end = ImVec2(cursorPos.x + ((i + 1) * segmentWidth), cursorPos.y + constants::sizes::COLOR_SCALE_HEIGHT);

                    // Get colors
                    ImColor col1 = ImColor(colors[i].x, colors[i].y, colors[i].z);
                    ImColor col2 = ImColor(colors[i+1].x, colors[i+1].y, colors[i+1].z);

                    // Draw the segment
                    draw_list->AddRectFilledMultiColor(start, end, col1, col2, col2, col1);
                }

                // Fill the space of the color bar so the text won't overlap it
                ImGui::Dummy(ImVec2(constants::sizes::COLOR_SCALE_WIDTH, constants::sizes::COLOR_SCALE_HEIGHT));

                // Range label
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
                ImGui::Text("Range: ");
                ImGui::PopStyleColor();
                ImGui::SameLine();
                ImGui::Text("[%.1f, %.1f] %s", range.first, range.second, this->app.scene->colorPalette->getUnitOfMeasure().c_str());
            } else {
                ImGui::Text("Color scale not set");
            }
            ImGui::End();
        }

        if (open != this->app.settings.isColorScaleActive()) this->app.settings.toggleColorScale();

    }

    void Gui::drawClusterConfiguration() {
        bool open = this->app.settings.isClusterConfigInfoActive();

        if (open) {
            ImGui::SetNextWindowPos(ImVec2(
                this->app.windowManager->getCurrentResolution().x * 0.5f,
                this->menuBarHeight + constants::sizes::PADDING),
                ImGuiCond_FirstUseEver,
                ImVec2(0.5f, 0.0f) // Set top - center pivot
            );

            ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;

            if (this->app.stateManager.getCurrentState() == AppState::EXPORT_IMAGE) windowFlags |= ImGuiWindowFlags_NoTitleBar; // In the screenshot the title bar must disappear

            ImGui::Begin(
                "Clustering Options",
                &open,
                windowFlags
            );
            ImGui::BeginTable("ClusterInfo", 2, ImGuiTableFlags_None);

            ImGui::TableSetupColumn("Labels", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Values", ImGuiTableColumnFlags_WidthFixed);

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Indent(10.0f);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            ImGui::Text("Veto Max Gap");
            ImGui::Spacing();
            ImGui::Text("Veto Min Entries");
            ImGui::Spacing();
            ImGui::Text("SciFi Max Gap");
            ImGui::Spacing();
            ImGui::Text("SciFi Min Entries");
            ImGui::Spacing();
            ImGui::Text("US Max Gap");
            ImGui::Spacing();
            ImGui::Text("US Min Entries");
            ImGui::Spacing();
            ImGui::Text("DS Max Gap");
            ImGui::Spacing();
            ImGui::Text("DS Min Entries");
            ImGui::PopStyleColor();
            ImGui::Unindent(10.0f);

            ImGui::TableSetColumnIndex(1);
            ClusterConfiguration* config = this->app.stateManager.getClusterConfiguration();
            ImGui::Text("%.2f", config->vetoMaxGap);
            ImGui::Spacing();
            ImGui::Text("%d", config->vetoMinHitInCluster);
            ImGui::Spacing();
            ImGui::Text("%.2f", config->sciFiMaxGap);
            ImGui::Spacing();
            ImGui::Text("%d", config->sciFiMinHitInCluster);
            ImGui::Spacing();
            ImGui::Text("%.2f", config->usMaxGap);
            ImGui::Spacing();
            ImGui::Text("%d", config->usMinHitInCluster);
            ImGui::Spacing();
            ImGui::Text("%.2f", config->dsMaxGap);
            ImGui::Spacing();
            ImGui::Text("%d", config->dsMinHitInCluster);

            ImGui::EndTable();
            ImGui::End();
        }

        if (open != this->app.settings.isClusterConfigInfoActive()) this->app.settings.toggleClusterConfigInfo();
    }
}

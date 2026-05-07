#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "rendering/camera/Camera.hpp"
#include "rendering/camera/Projection.hpp"
#include "core/WindowManager.hpp"
#include "ui/Gui.hpp"

namespace snd3D {
    class Viewport {
        public:
            Viewport(WindowManager& winMan, Gui& guiMan, bool _ortographic);
            void update();

            // GETTER
            bool isCameraChanged();
            bool isOrthographic();
            const glm::vec3 getCameraTarget() const;
            const glm::vec3 getCameraPosition() const;
            const glm::mat4& getProjectionMatrix() const;
            const glm::mat4& getViewMatrix() const;

            // MOVEMENTS
            void moveParallel(float deltaX, float deltaY);
            void movePerpendicular(float deltaX, float deltaY);
            void rotateByAngles(float deltaAngleX, float deltaAngleY);
            void rotateTrackball(glm::vec3 origin, glm::vec3 destination);

            void setDirection(Camera::Directions direction);
            void toggleProjectionType();
            void zoom(float offset);
            void reset();


        private:
            std::unique_ptr<Camera> camera;
            std::unique_ptr<Projection> projection;
            bool ortographic;

            // Owned by App, keep reference to spped up access
            WindowManager& windowManager;
            Gui& guiManager;

            glm::vec3 cursorToUnitSphere(int x, int y);
    };
}

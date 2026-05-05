#pragma once

#include <glm/glm.hpp>

namespace snd3D {
    class Camera {
        public:
            enum class Directions {
                ALIGN_X,
                ALIGN_X_NEG,
                ALIGN_Y,
                ALIGN_Y_NEG,
                ALIGN_Z,
                ALIGN_Z_NEG,
                ISOMETRIC1,
                ISOMETRIC2
            };

            Camera(glm::vec3 _position, glm::vec3 _target);
            const glm::mat4& getMatrix();
            const glm::vec3& getPosition();
            const glm::vec3& getTarget();
            void rotateAroundAxis(glm::vec3 axis, float angle);
            void rotateByAngles(float deltaAngleX, float deltaAngleY);
            void rotateTrackball(glm::vec3 origin, glm::vec3 destination);
            void moveParallel(float deltaX, float deltaY);
            void movePerpendicular(float deltaX, float deltaY);
            void zoom(float offset);
            void setDirection(Directions direction);
            bool isChanged();
            void reset();

        private:
            glm::vec3 position;  // Camera position in 3D space
            glm::vec3 target;    // Point the camera is looking at
            glm::vec3 worldUp;  // Up direction vector for the camera

            // Used to reset to default values
            glm::vec3 startPosition;
            glm::vec3 startTarget;
            glm::vec3 startWorldUp;

            glm::mat4 viewMatrix; // Buffered view matrix to not recompute it for each frame, but it has to be kept updated
            bool changed;
            void recomputeMatrix(); // Use it to recompute the buffered matrix, because it updates the changed flag
    };

}

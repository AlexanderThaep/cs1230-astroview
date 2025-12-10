#pragma once

#include <glm/glm.hpp>
#include <utils/sceneparser.h>

// A class representing a virtual camera.

// Feel free to make your own design choices for Camera class, the functions below are all optional / for your convenience.
// You can either implement and use these getters, or make your own design.
// If you decide to make your own design, feel free to delete these as TAs won't rely on them to grade your assignments.

class Camera {
private:
    glm::mat4 c_viewMatrix;
    glm::mat4 c_inverseViewMatrix;
    glm::mat4 c_projectionMatrix;
    glm::mat4 c_inverseProjectionMatrix;

    float c_aspectRatio;
    float c_heightAngle;
    float c_tanHalfVAngle;
    float c_tanHalfHAngle;

    glm::vec3 c_position;

public:
    Camera(SceneCameraData &cameraData);
    void update(SceneCameraData &cameraData, float ar);
    void updateView(SceneCameraData &cameraData);

    // Returns the view matrix for the current camera settings.
    // You might also want to define another function that return the inverse of the view matrix.
    glm::mat4 getViewMatrix() const;
    glm::mat4 getInverseViewMatrix() const;

    // Returns the projection matrix for the current camera settings.
    glm::mat4 getProjectionMatrix() const;
    glm::mat4 getInverseProjectionMatrix() const;

    // Returns the aspect ratio of the camera.
    float getAspectRatio() const;

    // Returns the height angle of the camera in RADIANS.
    float getHeightAngle() const;

    // Get camera position in world space
    glm::vec3 getPosition() const;

    // Camera manipulation functions
    void moveMedial(SceneCameraData &cameraData, float delta);
    void moveLateral(SceneCameraData &cameraData, float delta);
    void moveVertical(SceneCameraData &cameraData, float delta);
    void rotate(SceneCameraData &cameraData, float deltaX, float deltaY);
};

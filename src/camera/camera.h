#ifndef CAMERA_H
#define CAMERA_H
#include "utils/scenedata.h"
#include <glm/glm.hpp>


class Camera {
public:
    // Returns the view matrix for the current camera settings.
    glm::mat4 getViewMatrix() const;

    //Returns the inverse of the view matrix
    glm::mat4 getInverseViewMatrix() const;

    //Returns the projection matrix
    glm::mat4 getProjectionMatrix() const;

    //Returns the inverse of the projective matrix
    glm::mat4 getInverseProjectionMatrix() const;

    // Returns the height angle of the camera in RADIANS.
    float getHeightAngle() const;

    // Returns the width angle of the camera in RADIANS
    float getWidthAngle() const;

    //Returns the aspect ratio
    float getAspectRatio() const;

    //Updatest eh new and far
    void updateNearAndFarPlanes(float near, float far);

    //The scene camera data
    SceneCameraData cameraData;

    //The current near and far planes
    float near = -1;
    float far = -1;

    //The viewPlane with and height
    float viewPlaneHeight = -1;
    float viewPlaneWidth = -1;

};

#endif

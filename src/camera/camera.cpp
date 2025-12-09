#include "camera.h"
#include "settings.h"
#include <utils/sceneparser.h>

Camera::Camera(SceneCameraData &cameraData) {};

void Camera::moveMedial(SceneCameraData &cameraData, float delta)
{
    cameraData.pos += cameraData.look * delta;
    updateView(cameraData);
}

void Camera::moveLateral(SceneCameraData &cameraData, float delta)
{
    glm::vec4 right = glm::vec4(glm::normalize(glm::cross(glm::vec3(cameraData.look), glm::vec3(cameraData.up))), 0.0f);

    cameraData.pos += right * delta;
    updateView(cameraData);
}

void Camera::moveVertical(SceneCameraData &cameraData, float delta)
{
    cameraData.pos += glm::vec4(0.0f, 1.0f, 0.0f, 0.0f) * delta;
    updateView(cameraData);
}

void Camera::rotate(SceneCameraData &cameraData, float deltaX, float deltaY)
{
    const glm::vec3 yAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 look = glm::vec3(cameraData.look);
    glm::vec3 up = glm::vec3(cameraData.up);
    glm::vec3 right = glm::normalize(glm::cross(look, up));

    look =
        glm::normalize(look * glm::cos(deltaY)
                    + glm::cross(right, look) * glm::sin(deltaY)
                    + right * glm::dot(right, look) * (1.0f - glm::cos(deltaY)));

    up =
        glm::normalize(up * glm::cos(deltaY)
                    + glm::cross(right, up) * glm::sin(deltaY)
                    + right * glm::dot(right, up) * (1.0f - glm::cos(deltaY)));

    cameraData.look =
        glm::normalize(glm::vec4(look * glm::cos(deltaX)
                    + glm::cross(yAxis, look) * glm::sin(deltaX)
                    + yAxis * glm::dot(yAxis, look) * (1.0f - glm::cos(deltaX)), 0.0f));

    cameraData.up =
        glm::normalize(glm::vec4(up * glm::cos(deltaX)
                    + glm::cross(yAxis, up) * glm::sin(deltaX)
                    + yAxis * glm::dot(yAxis, up) * (1.0f - glm::cos(deltaX)), 0.0f));

    updateView(cameraData);
}

void Camera::updateView(SceneCameraData &cameraData)
{
    glm::mat3 lpu = glm::mat3(
        glm::vec3(cameraData.look),
        glm::vec3(cameraData.pos),
        glm::vec3(cameraData.up));

    glm::vec3 w = -glm::normalize(lpu[0]);
    glm::vec3 v = glm::normalize(lpu[2] - (glm::dot(lpu[2], w) * w));
    glm::vec3 u = glm::cross(v, w);

    glm::mat4 rotation = glm::mat4(
        glm::vec4(u.x, v.x, w.x, 0.0f),
        glm::vec4(u.y, v.y, w.y, 0.0f),
        glm::vec4(u.z, v.z, w.z, 0.0f),
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );

    glm::mat4 translation = glm::mat4(
        glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
        glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
        glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
        glm::vec4(-lpu[1].x, -lpu[1].y, -lpu[1].z, 1.0f)
    );

    this->c_position = lpu[1];

    this->c_viewMatrix = rotation * translation;
    this->c_inverseViewMatrix = glm::inverse(this->c_viewMatrix);
}

void Camera::update(SceneCameraData &cameraData, float ar)
{
    this->c_aspectRatio = ar;
    this->c_heightAngle = cameraData.heightAngle;
    this->c_tanHalfVAngle = glm::tan(this->c_heightAngle / 2.0f);
    this->c_tanHalfHAngle = this->c_tanHalfVAngle * ar;

    float far = settings.farPlane;
    float near = settings.nearPlane;
    float c = -near / far;

    // Projection
    glm::mat4 z_scale = glm::mat4(
        glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
        glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
        glm::vec4(0.0f, 0.0f, -2.0f, 0.0f),
        glm::vec4(0.0f, 0.0f, -1.0f, 1.0f)
    );

    glm::mat4 unhinging = glm::mat4(
        glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
        glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
        glm::vec4(0.0f, 0.0f, 1.0f / (1.0f + c), -1.0f),
        glm::vec4(0.0f, 0.0f, -c / (1.0f + c), 0.0f)
    );

    glm::mat4 projection = glm::mat4(
        glm::vec4(1.0f / (far * this->c_tanHalfHAngle), 0.0f, 0.0f, 0.0f),
        glm::vec4(0.0f, 1.0f / (far * this->c_tanHalfVAngle), 0.0f, 0.0f),
        glm::vec4(0.0f, 0.0f, 1.0f / far, 0.0f),
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );

    this->c_projectionMatrix = z_scale * unhinging * projection;
}

glm::vec3 Camera::getPosition() const
{
    return this->c_position;
}

glm::mat4 Camera::getViewMatrix() const
{
    return this->c_viewMatrix;
}

glm::mat4 Camera::getInverseViewMatrix() const
{
    return this->c_inverseViewMatrix;
}

glm::mat4 Camera::getProjectionMatrix() const
{
    return this->c_projectionMatrix;
}

float Camera::getAspectRatio() const
{
    return this->c_aspectRatio;
}

float Camera::getHeightAngle() const
{
    return this->c_heightAngle;
}

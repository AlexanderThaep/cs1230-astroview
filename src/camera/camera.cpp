#include "camera.h"

glm::mat4 Camera::getViewMatrix() const {
    glm::vec3 pos = glm::vec3(cameraData.pos);
    glm::vec3 look = glm::vec3(cameraData.look);;
    glm::vec3 up = glm::vec3(cameraData.up);

    //find u, v, w
    glm::vec3 w = -glm::normalize(look);
    glm::vec3 v = glm::normalize(up - glm::dot(up, w) * w);
    glm::vec3 u = glm::cross(v, w);

    //construct matrices
    glm::mat4 m_translate = glm::mat4(glm::vec4(1., 0., 0., 0.),
                                      glm::vec4(0., 1., 0., 0.),
                                      glm::vec4(0., 0., 1., 0.),
                                      glm::vec4(-pos[0], -pos[1], -pos[2], 1.));

    glm::mat4 m_rotate = glm::mat4(u[0], v[0], w[0], 0.,
                                   u[1], v[1], w[1], 0.,
                                   u[2], v[2], w[2], 0.,
                                   0., 0., 0., 1.);

    return m_rotate * m_translate;
}

glm::mat4 Camera::getInverseViewMatrix() const {
    return glm::inverse(getViewMatrix());
}

//Returns the projection matrix
glm::mat4 Camera::getProjectionMatrix() const {
    float theta_w = getWidthAngle();
    float theta_h = getHeightAngle();
    float c = -near/far;

    glm::mat4 scale = glm::mat4( 1.f / (far * tan(theta_w / 2.f)), 0, 0, 0,
                                0, 1.f / (far * tan(theta_h / 2.f)), 0, 0,
                                0, 0, 1.f / far, 0,
                                0, 0, 0, 1);

    glm::mat4 mpp = glm::mat4(1, 0, 0, 0,
                              0, 1, 0, 0,
                              0, 0, 1.f / (1.f + c), -1.f,
                              0, 0, -c / (1.f + c), 0.f);

    glm::mat4 transform = glm::mat4(1, 0, 0, 0,
                                    0, 1, 0, 0,
                                    0, 0, -2, 0,
                                    0, 0, -1, 1);

    return transform * mpp * scale;
}

//Returns the inverse of the projective matrix
glm::mat4 Camera::getInverseProjectionMatrix() const {
    return glm::inverse(getProjectionMatrix());
}

void Camera::updateNearAndFarPlanes(float near, float far) {
    this->near = near;
    this->far = far;
}

float Camera::getAspectRatio() const {
    return viewPlaneWidth / viewPlaneHeight;
}

float Camera::getHeightAngle() const {
    return cameraData.heightAngle;
}

float Camera::getWidthAngle() const {
    return 2.0f * atan(getAspectRatio() * tan(getHeightAngle() / 2.0f));
}

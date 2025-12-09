#include "Cylinder.h"

void Cylinder::updateParams(int param1, int param2) {
    if (param2 < 3) param2 = 3;

    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
}

void Cylinder::makeTopTile(glm::vec3 topLeft,
                       glm::vec3 topRight,
                       glm::vec3 bottomLeft,
                       glm::vec3 bottomRight) {

    glm::vec3 normal = glm::vec3(0.0f, -1.0f, 0.0f);

    insertVec3(m_vertexData, glm::vec3(topLeft.x, topLeft.y, topLeft.z));
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, glm::vec3(bottomLeft.x, bottomLeft.y, bottomLeft.z));
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, glm::vec3(bottomRight.x, bottomRight.y, bottomRight.z));
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, glm::vec3(topLeft.x, topLeft.y, topLeft.z));
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, glm::vec3(bottomRight.x, bottomRight.y, bottomRight.z));
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, glm::vec3(topRight.x, topRight.y, topRight.z));
    insertVec3(m_vertexData, normal);
}

void Cylinder::makeBotTile(glm::vec3 topLeft,
                           glm::vec3 topRight,
                           glm::vec3 bottomLeft,
                           glm::vec3 bottomRight) {

    glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);

    insertVec3(m_vertexData, glm::vec3(topRight.x, topRight.y, topRight.z));
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, glm::vec3(bottomRight.x, bottomRight.y, bottomRight.z));
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, glm::vec3(bottomLeft.x, bottomLeft.y, bottomLeft.z));
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, glm::vec3(topRight.x, topRight.y, topRight.z));
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, glm::vec3(bottomLeft.x, bottomLeft.y, bottomLeft.z));
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, glm::vec3(topLeft.x, topLeft.y, topLeft.z));
    insertVec3(m_vertexData, normal);
}

static inline glm::vec3 zeroYNorm(glm::vec3 v, glm::vec3 o) {
    glm::vec3 n = glm::normalize(v - o);
    n.y = 0.0f;
    return n;
}

void Cylinder::makeMidTile(glm::vec3 topLeft,
                           glm::vec3 topRight,
                           glm::vec3 bottomLeft,
                           glm::vec3 bottomRight) {

    glm::vec3 origin = glm::vec3(0.0f);

    insertVec3(m_vertexData, glm::vec3(topLeft.x, topLeft.y, topLeft.z));
    insertVec3(m_vertexData, zeroYNorm(topLeft, origin));
    insertVec3(m_vertexData, glm::vec3(bottomLeft.x, bottomLeft.y, bottomLeft.z));
    insertVec3(m_vertexData, zeroYNorm(bottomLeft, origin));
    insertVec3(m_vertexData, glm::vec3(bottomRight.x, bottomRight.y, bottomRight.z));
    insertVec3(m_vertexData, zeroYNorm(bottomRight, origin));
    insertVec3(m_vertexData, glm::vec3(topLeft.x, topLeft.y, topLeft.z));
    insertVec3(m_vertexData, zeroYNorm(topLeft, origin));
    insertVec3(m_vertexData, glm::vec3(bottomRight.x, bottomRight.y, bottomRight.z));
    insertVec3(m_vertexData, zeroYNorm(bottomRight, origin));
    insertVec3(m_vertexData, glm::vec3(topRight.x, topRight.y, topRight.z));
    insertVec3(m_vertexData, zeroYNorm(topRight, origin));
}

void Cylinder::makeTopSlice(float currentTheta, float nextTheta){
    float delta = 0.5f / m_param1;
    float y = -0.5f;

    for (int i = 0; i < m_param1; i++) {

        float r = delta * i;
        float next_r = (r + delta);
        float x1 = glm::cos(currentTheta);
        float z1 = glm::sin(currentTheta);

        float x2 = glm::cos(nextTheta);
        float z2 = glm::sin(nextTheta);

        glm::vec3 topLeft = glm::vec3(r * x1, y, r * z1);
        glm::vec3 topRight = topLeft;

        glm::vec3 bottomLeft = glm::vec3(next_r * x1, y, next_r * z1);
        glm::vec3 bottomRight = glm::vec3(next_r * x2, y, next_r * z2);

        makeTopTile(topLeft, topRight, bottomLeft, bottomRight);
    }

    for (int i = 1; i < m_param1; i++) {

        float r = delta * i;
        float next_r = (r + delta);
        float x1 = glm::cos(currentTheta);
        float z1 = glm::sin(currentTheta);

        float x2 = glm::cos(nextTheta);
        float z2 = glm::sin(nextTheta);

        glm::vec3 topLeft = glm::vec3(r * x1, y, r * z1);
        glm::vec3 topRight = topLeft;

        glm::vec3 bottomLeft = glm::vec3(next_r * x2, y, next_r * z2);
        glm::vec3 bottomRight = glm::vec3(r * x2, y, r * z2);

        makeTopTile(topLeft, topRight, bottomLeft, bottomRight);
    }
}

void Cylinder::makeBotSlice(float currentTheta, float nextTheta){
    float delta = 0.5f / m_param1;
    float y = 0.5f;

    for (int i = 0; i < m_param1; i++) {

        float r = delta * i;
        float next_r = (r + delta);
        float x1 = glm::cos(currentTheta);
        float z1 = glm::sin(currentTheta);

        float x2 = glm::cos(nextTheta);
        float z2 = glm::sin(nextTheta);

        glm::vec3 topLeft = glm::vec3(r * x1, y, r * z1);
        glm::vec3 topRight = topLeft;

        glm::vec3 bottomLeft = glm::vec3(next_r * x1, y, next_r * z1);
        glm::vec3 bottomRight = glm::vec3(next_r * x2, y, next_r * z2);

        makeBotTile(topLeft, topRight, bottomLeft, bottomRight);
    }

    for (int i = 1; i < m_param1; i++) {

        float r = delta * i;
        float next_r = (r + delta);
        float x1 = glm::cos(currentTheta);
        float z1 = glm::sin(currentTheta);

        float x2 = glm::cos(nextTheta);
        float z2 = glm::sin(nextTheta);

        glm::vec3 topLeft = glm::vec3(r * x1, y, r * z1);
        glm::vec3 topRight = topLeft;

        glm::vec3 bottomLeft = glm::vec3(next_r * x2, y, next_r * z2);
        glm::vec3 bottomRight = glm::vec3(r * x2, y, r * z2);

        makeBotTile(topLeft, topRight, bottomLeft, bottomRight);
    }
}

void Cylinder::makeMidSlice(float currentTheta, float nextTheta){
    float delta = 1.0f / m_param1;
    float y1 = 0.5f;
    float y2 = y1 - delta;

    float r = 0.5f;

    for (int i = 0; i < m_param1; i++) {
        float x1 = glm::cos(currentTheta);
        float z1 = glm::sin(currentTheta);

        float x2 = glm::cos(nextTheta);
        float z2 = glm::sin(nextTheta);

        glm::vec3 topRight = glm::vec3(r * x1, y1, r * z1);
        glm::vec3 topLeft = glm::vec3(r * x2, y1, r * z2);

        glm::vec3 bottomRight = glm::vec3(r * x1, y2, r * z1);
        glm::vec3 bottomLeft = glm::vec3(r * x2, y2, r * z2);

        makeMidTile(topLeft, topRight, bottomLeft, bottomRight);

        y1 -= delta;
        y2 = y1 - delta;
    }
}

void Cylinder::makeWedge(float currentTheta, float nextTheta) {
    makeTopSlice(currentTheta, nextTheta);
    makeBotSlice(currentTheta, nextTheta);
    makeMidSlice(currentTheta, nextTheta);
}

void Cylinder::setVertexData() {
    // TODO for Project 5: Lights, Camera
    float thetaStep = glm::radians(360.f / m_param2);

    for (int i = 0; i < m_param2; i++) {
        float currentTheta = i * thetaStep;
        float nextTheta = (i + 1) * thetaStep;
        makeWedge(currentTheta, nextTheta);
    }
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cylinder::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

#include "Cone.h"

void Cone::updateParams(int param1, int param2) {
    if (param2 < 3) param2 = 3;
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
}

glm::vec3 Cone::calcNorm(glm::vec3& pt) {
    float xNorm = (2 * pt.x);
    float yNorm = -(1.f/4.f) * (2.f * pt.y - 1.f);
    float zNorm = (2 * pt.z);

    return glm::normalize(glm::vec3{ xNorm, yNorm, zNorm });
}

void Cone::makeTip(float currentTheta, float nextTheta) {
    float base_r = 0.5f;
    float next_r = base_r / m_param1;

    float delta = 1.0f / m_param1;
    float y1 = 0.5f;
    float y2 = y1 - delta;

    float y = -0.5f;
    float x1 = glm::cos(currentTheta);
    float z1 = glm::sin(currentTheta);

    float x2 = glm::cos(nextTheta);
    float z2 = glm::sin(nextTheta);

    glm::vec v1 = glm::vec3(base_r * x1, y, base_r * z1);
    glm::vec v2 = glm::vec3(base_r * x2, y, base_r * z2);
    glm::vec3 n1 = calcNorm(v1);
    glm::vec3 n2 = calcNorm(v2);

    glm::vec3 topRight = glm::vec3(0.0f, y1, 0.0f);
    glm::vec3 topLeft = glm::vec3(0.0f, y1, 0.0f);

    glm::vec3 bottomRight = glm::vec3(next_r * x1, y2, next_r * z1);
    glm::vec3 bottomLeft = glm::vec3(next_r * x2, y2, next_r * z2);

    glm::vec3 n = (n1 + n2) * 0.5f;

    insertVec3(m_vertexData, glm::vec3(topLeft.x, topLeft.y, topLeft.z));
    insertVec3(m_vertexData, n);
    insertVec3(m_vertexData, glm::vec3(bottomLeft.x, bottomLeft.y, bottomLeft.z));
    insertVec3(m_vertexData, calcNorm(bottomLeft));
    insertVec3(m_vertexData, glm::vec3(bottomRight.x, bottomRight.y, bottomRight.z));
    insertVec3(m_vertexData, calcNorm(bottomRight));
    insertVec3(m_vertexData, glm::vec3(topLeft.x, topLeft.y, topLeft.z));
    insertVec3(m_vertexData, n);
    insertVec3(m_vertexData, glm::vec3(bottomRight.x, bottomRight.y, bottomRight.z));
    insertVec3(m_vertexData, calcNorm(bottomRight));
    insertVec3(m_vertexData, glm::vec3(topRight.x, topRight.y, topRight.z));
    insertVec3(m_vertexData, n);
}

void Cone::makeConeTile(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight) {

    insertVec3(m_vertexData, glm::vec3(topLeft.x, topLeft.y, topLeft.z));
    insertVec3(m_vertexData, calcNorm(topLeft));
    insertVec3(m_vertexData, glm::vec3(bottomLeft.x, bottomLeft.y, bottomLeft.z));
    insertVec3(m_vertexData, calcNorm(bottomLeft));
    insertVec3(m_vertexData, glm::vec3(bottomRight.x, bottomRight.y, bottomRight.z));
    insertVec3(m_vertexData, calcNorm(bottomRight));
    insertVec3(m_vertexData, glm::vec3(topLeft.x, topLeft.y, topLeft.z));
    insertVec3(m_vertexData, calcNorm(topLeft));
    insertVec3(m_vertexData, glm::vec3(bottomRight.x, bottomRight.y, bottomRight.z));
    insertVec3(m_vertexData, calcNorm(bottomRight));
    insertVec3(m_vertexData, glm::vec3(topRight.x, topRight.y, topRight.z));
    insertVec3(m_vertexData, calcNorm(topRight));
}

void Cone::makeCapTile(glm::vec3 topLeft,
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

// Task 8: create function(s) to make tiles which you can call later on
// Note: Consider your makeTile() functions from Sphere and Cube

void Cone::makeCapSlice(float currentTheta, float nextTheta){
    // Task 8: create a slice of the cap face using your
    //         make tile function(s)
    // Note: think about how param 1 comes into play here!

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

        makeCapTile(topLeft, topRight, bottomLeft, bottomRight);
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

        makeCapTile(topLeft, topRight, bottomLeft, bottomRight);
    }
}

void Cone::makeSlopeSlice(float currentTheta, float nextTheta){
    // Task 9: create a single sloped face using your make
    //         tile function(s)
    // Note: think about how param 1 comes into play here!

    float delta = 1.0f / m_param1;
    float y1 = 0.5f;
    float y2 = y1 - delta;

    float delta_r = 0.5f / m_param1;

    makeTip(currentTheta, nextTheta);

    y1 -= delta;
    y2 = y1 - delta;

    for (int i = 1; i < m_param1; i++) {
        float r = delta_r * i;
        float next_r = (r + delta_r);

        float x1 = glm::cos(currentTheta);
        float z1 = glm::sin(currentTheta);

        float x2 = glm::cos(nextTheta);
        float z2 = glm::sin(nextTheta);

        glm::vec3 topRight = glm::vec3(r * x1, y1, r * z1);
        glm::vec3 topLeft = glm::vec3(r * x2, y1, r * z2);

        glm::vec3 bottomRight = glm::vec3(next_r * x1, y2, next_r * z1);
        glm::vec3 bottomLeft = glm::vec3(next_r * x2, y2, next_r * z2);

        makeConeTile(topLeft, topRight, bottomLeft, bottomRight);

        y1 -= delta;
        y2 = y1 - delta;
    }
}

void Cone::makeWedge(float currentTheta, float nextTheta) {
    // Task 10: create a single wedge of the Cone using the
    //          makeCapSlice() and makeSlopeSlice() functions you
    //          implemented in Task 8

    makeCapSlice(currentTheta, nextTheta);
    makeSlopeSlice(currentTheta, nextTheta);
}

void Cone::setVertexData() {
    // Task 10: create a full cone using the makeWedge() function you
    //          just implemented
    // Note: think about how param 2 comes into play here!

    float thetaStep = glm::radians(360.f / m_param2);

    for (int i = 0; i < m_param2; i++) {
        float currentTheta = i * thetaStep;
        float nextTheta = (i + 1) * thetaStep;
        makeWedge(currentTheta, nextTheta);
    }
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cone::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

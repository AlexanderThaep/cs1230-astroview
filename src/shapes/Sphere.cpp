#include "Sphere.h"

void Sphere::updateParams(int param1, int param2) {
    if (param1 < 2) param1 = 2;
    if (param2 < 3) param2 = 3;
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
}

void Sphere::makeTile(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight) {
    // Task 5: Implement the makeTile() function for a Sphere
    // Note: this function is very similar to the makeTile() function for Cube,
    //       but the normals are calculated in a different way!

    glm::vec3 origin = glm::vec3(0.0f);

    insertVec3(m_vertexData, glm::vec3(topLeft.x, topLeft.y, topLeft.z));
    insertVec3(m_vertexData, glm::normalize(topLeft - origin));
    insertVec3(m_vertexData, glm::vec3(bottomLeft.x, bottomLeft.y, bottomLeft.z));
    insertVec3(m_vertexData, glm::normalize(bottomLeft - origin));
    insertVec3(m_vertexData, glm::vec3(bottomRight.x, bottomRight.y, bottomRight.z));
    insertVec3(m_vertexData, glm::normalize(bottomRight - origin));
    insertVec3(m_vertexData, glm::vec3(topLeft.x, topLeft.y, topLeft.z));
    insertVec3(m_vertexData, glm::normalize(topLeft - origin));
    insertVec3(m_vertexData, glm::vec3(bottomRight.x, bottomRight.y, bottomRight.z));
    insertVec3(m_vertexData, glm::normalize(bottomRight - origin));
    insertVec3(m_vertexData, glm::vec3(topRight.x, topRight.y, topRight.z));
    insertVec3(m_vertexData, glm::normalize(topRight - origin));
}

void Sphere::makeWedge(float currentTheta, float nextTheta) {
    // Task 6: create a single wedge of the sphere using the
    //         makeTile() function you implemented in Task 5
    // Note: think about how param 1 comes into play here!

    float delta = glm::radians(180.0f / m_param1);
    float r = 0.5f;

    for (int i = 0; i < m_param1; i++) {
        float phi = delta * i;
        glm::vec3 bottomLeft = glm::vec3(r * glm::sin(phi + delta) * glm::cos(currentTheta), r * glm::cos(phi + delta), -r * glm::sin(phi + delta) * glm::sin(currentTheta));
        glm::vec3 bottomRight = glm::vec3(r * glm::sin(phi + delta) * glm::cos(nextTheta), r * glm::cos(phi + delta), -r * glm::sin(phi + delta) * glm::sin(nextTheta));
        glm::vec3 topLeft = glm::vec3(r * glm::sin(phi) * glm::cos(currentTheta), r * glm::cos(phi), -r * glm::sin(phi) * glm::sin(currentTheta));
        glm::vec3 topRight = glm::vec3(r * glm::sin(phi) * glm::cos(nextTheta), r * glm::cos(phi), -r * glm::sin(phi) * glm::sin(nextTheta));

        makeTile(topLeft, topRight, bottomLeft, bottomRight);
    }
}

void Sphere::makeSphere() {
    // Task 7: create a full sphere using the makeWedge() function you
    //         implemented in Task 6
    // Note: think about how param 2 comes into play here!

    float thetaStep = glm::radians(360.f / m_param2);

    for (int i = 0; i < m_param2; i++) {
        float currentTheta = i * thetaStep;
        float nextTheta = (i + 1) * thetaStep;
        makeWedge(currentTheta, nextTheta);
    }
}

void Sphere::setVertexData() {
    // Uncomment these lines to make a wedge for Task 6, then comment them out for Task 7:

    // float thetaStep = glm::radians(360.f / m_param2);
    // float currentTheta = 0 * thetaStep;
    // float nextTheta = 1 * thetaStep;
    // makeWedge(currentTheta, nextTheta);

    // Uncomment these lines to make sphere for Task 7:

    makeSphere();
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Sphere::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

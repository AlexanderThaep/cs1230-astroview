#include "Cube.h"

void Cube::updateParams(int param1) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    setVertexData();
}

void Cube::makeTile(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {
    // Task 2: create a tile (i.e. 2 triangles) based on 4 given points.

    glm::vec3 normal = glm::normalize(glm::cross(bottomLeft - topLeft, topRight - topLeft));

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

void Cube::makeFace(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {
    // Task 3: create a single side of the cube out of the 4
    //         given points and makeTile()
    // Note: think about how param 1 affects the number of triangles on
    //       the face of the cube

    float step = 1.0f / m_param1;
    glm::vec3 unitX = glm::normalize(topRight - topLeft) * step;
    glm::vec3 unitY = glm::normalize(bottomLeft - topLeft) * step;

    for (int i = 0; i < m_param1; i++) {
        glm::vec3 transX = unitX * (float)i;
        for (int j = 0; j < m_param1; j++) {
            glm::vec3 transY = unitY * (float)j;
            glm::vec3 tl = topLeft + transX + transY;
            glm::vec3 tr = topLeft + (transX + unitX) + transY;
            glm::vec3 bl = topLeft + transX + (transY + unitY);
            glm::vec3 br = topLeft + transX + transY + unitX + unitY;

            makeTile(tl, tr, bl, br);
        }
    }

    // glm::vec3 center = (topLeft + topRight + bottomLeft + bottomRight) * 0.25f;
    // makeFace(topLeft, (topLeft + topRight) * 0.5f, (bottomLeft + topLeft) * 0.5f, center);
    // makeFace((topLeft + topRight) * 0.5f, topRight, center, (bottomRight + topRight) * 0.5f);
    // makeFace((bottomLeft + topLeft) * 0.5f, center, bottomLeft, (bottomRight + bottomLeft) * 0.5f);
    // makeFace(center, (bottomRight + topRight) * 0.5f, (bottomRight + bottomLeft) * 0.5f, bottomRight);
}

void Cube::setVertexData() {
    // Uncomment these lines for Task 2, then comment them out for Task 3:

    // makeTile(glm::vec3(-0.5f,  0.5f, 0.5f),
    //          glm::vec3( 0.5f,  0.5f, 0.5f),
    //          glm::vec3(-0.5f, -0.5f, 0.5f),
    //          glm::vec3( 0.5f, -0.5f, 0.5f));

    // Uncomment these lines for Task 3:

    // makeFace(glm::vec3(-0.5f,  0.5f, 0.5f),
    //          glm::vec3( 0.5f,  0.5f, 0.5f),
    //          glm::vec3(-0.5f, -0.5f, 0.5f),
    //          glm::vec3( 0.5f, -0.5f, 0.5f),
    //          m_param1);

    // Task 4: Use the makeFace() function to make all 6 sides of the cube

    makeFace(glm::vec3(-0.5f,  0.5f, 0.5f),
             glm::vec3( 0.5f,  0.5f, 0.5f),
             glm::vec3(-0.5f, -0.5f, 0.5f),
             glm::vec3( 0.5f, -0.5f, 0.5f));

    makeFace(glm::vec3( 0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f));

    makeFace(glm::vec3(-0.5f, -0.5f,  0.5f),
             glm::vec3( 0.5f, -0.5f,  0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f));

    makeFace(glm::vec3( 0.5f,  0.5f,  0.5f),
             glm::vec3(-0.5f,  0.5f,  0.5f),
             glm::vec3( 0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f, -0.5f));

    makeFace(glm::vec3( 0.5f, -0.5f,  0.5f),
             glm::vec3( 0.5f,  0.5f,  0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f),
             glm::vec3( 0.5f,  0.5f, -0.5f));

    makeFace(glm::vec3(-0.5f,  0.5f,  0.5f),
             glm::vec3(-0.5f, -0.5f,  0.5f),
             glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f));
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cube::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

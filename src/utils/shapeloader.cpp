#include "realtime.h"
#include "settings.h"
#include "utils/sceneparser.h"

#include "shapeloader.h"

static inline float remap(float x, float a, float b, float c, float d)
{
    return c + (x - a) * (d - c) / (b - a);
}

void loadShape(RenderShapeData &o)
{
    std::vector<float> verts;

    switch (o.primitive.type) {
    case PrimitiveType::PRIMITIVE_SPHERE:
    {
        break;
    }
    case PrimitiveType::PRIMITIVE_CONE:
    {
        break;
    }
    case PrimitiveType::PRIMITIVE_CYLINDER:
    {
        break;
    }
    case PrimitiveType::PRIMITIVE_CUBE:
    {
        break;
    }
    case PrimitiveType::PRIMITIVE_SDF: break;
    case PrimitiveType::PRIMITIVE_SBH: break;
    }

    GLuint vbo;
    GLuint vao;

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 verts.size() * sizeof(float),
                 verts.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(0 * sizeof(GLfloat)));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    o.vbo = vbo;
    o.vao = vao;
}

void updateShapes(RenderData &rend)
{
    for (RenderShapeData &o : rend.shapes) loadShape(o);
}

void clearShapes(RenderData &rend)
{
    for (RenderShapeData &o : rend.shapes) {
        glDeleteBuffers(1, &o.vbo);
        glDeleteBuffers(1, &o.vao);
    }
    rend.shapes.clear();
    rend.lights.clear();
}

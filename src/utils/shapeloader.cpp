#include "objparser.h"
#include "realtime.h"
#include "settings.h"
#include "utils/sceneparser.h"

#include <shapes/shapes.h>
#include "shapeloader.h"

static inline float remap(float x, float a, float b, float c, float d)
{
    return c + (x - a) * (d - c) / (b - a);
}

void loadShape(RenderShapeData &o, int param1, int param2)
{
    std::vector<float> verts;

    o.param1 = param1;
    o.param2 = param2;

    switch (o.primitive.type) {
    case PrimitiveType::PRIMITIVE_SPHERE:
    {
        std::unique_ptr<Sphere> sphere = std::make_unique<Sphere>();
        sphere->updateParams(param1, param2);
        verts = sphere->generateShape();
        o.vertexCount = verts.size() / 2;
        break;
    }
    case PrimitiveType::PRIMITIVE_CONE:
    {
        std::unique_ptr<Cone> cone = std::make_unique<Cone>();
        cone->updateParams(param1, param2);
        verts = cone->generateShape();
        o.vertexCount = verts.size() / 2;
        break;
    }
    case PrimitiveType::PRIMITIVE_CYLINDER:
    {
        std::unique_ptr<Cylinder> cylinder = std::make_unique<Cylinder>();
        cylinder->updateParams(param1, param2);
        verts = cylinder->generateShape();
        o.vertexCount = verts.size() / 2;
        break;
    }
    case PrimitiveType::PRIMITIVE_CUBE:
    {
        std::unique_ptr<Cube> cube = std::make_unique<Cube>();
        cube->updateParams(param1);
        verts = cube->generateShape();
        o.vertexCount = verts.size() / 2;
        break;
    }
    case PrimitiveType::PRIMITIVE_TRI:
    {
        std::unique_ptr<Triangle> triangle = std::make_unique<Triangle>();
        triangle->updateParams();
        verts = triangle->generateShape();
        o.vertexCount = verts.size() / 2;
        break;
    }
    case PrimitiveType::PRIMITIVE_MESH:
        std::vector<OBJParser::Tri> tris;
        OBJParser *parser = new OBJParser(o);
        parser->parse(o.primitive.meshfile, tris);

        for (OBJParser::Tri t : tris) {
            verts.push_back(t.points[0][0]);
            verts.push_back(t.points[0][1]);
            verts.push_back(t.points[0][2]);

            verts.push_back((t.normal[0]));
            verts.push_back((t.normal[1]));
            verts.push_back((t.normal[2]));

            verts.push_back(t.points[1][0]);
            verts.push_back(t.points[1][1]);
            verts.push_back(t.points[1][2]);

            verts.push_back((t.normal[0]));
            verts.push_back((t.normal[1]));
            verts.push_back((t.normal[2]));

            verts.push_back(t.points[2][0]);
            verts.push_back(t.points[2][1]);
            verts.push_back(t.points[2][2]);

            verts.push_back((t.normal[0]));
            verts.push_back((t.normal[1]));
            verts.push_back((t.normal[2]));
        }

        o.vertexCount = tris.size() * 9;

        tris.clear();
        delete parser;

        break;
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

void loadLOD(RenderShapeData &o, glm::vec3 camPos)
{
    float distance = glm::length(o.worldPos - camPos);
    float lod_min = settings.farPlane * LOD_MIN_PERCENT;

    if (distance < lod_min) return;
    int p1 = settings.shapeParameter1 * remap(distance, lod_min, settings.farPlane, 1.0f, 0.0f);
    int p2 = settings.shapeParameter2 * remap(distance, lod_min, settings.farPlane, 1.0f, 0.0f);

    if (p1 != o.param1 || p2 != o.param2) {
        o.param1 = p1;
        o.param2 = p2;

        loadShape(o, o.param1, o.param2);
    }
}

void updateShapes(RenderData &rend, bool shouldLOD)
{
    if (shouldLOD) {
        for (RenderShapeData &o : rend.shapes) loadLOD(o, rend.cameraData.pos);
        return;
    }
    for (RenderShapeData &o : rend.shapes) loadShape(o, settings.shapeParameter1, settings.shapeParameter2);
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

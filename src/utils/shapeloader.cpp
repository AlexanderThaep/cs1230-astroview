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
    switch (o.primitive.type) {
    case PrimitiveType::PRIMITIVE_SBH: break;
    default:
        QString texFile = QString::fromStdString(o.primitive.material.textureMap.filename);
        if (!texFile.isEmpty()) o.hasTexture = true;
        break;
    }
}

void updateShapes(RenderData &rend)
{
    for (RenderShapeData &o : rend.shapes) loadShape(o);
}

void clearShapes(RenderData &rend)
{
    rend.shapes.clear();
    rend.lights.clear();
}

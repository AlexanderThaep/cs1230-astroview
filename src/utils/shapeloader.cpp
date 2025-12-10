#include "realtime.h"
#include "settings.h"
#include "utils/sceneparser.h"

#include "shapeloader.h"

static inline float remap(float x, float a, float b, float c, float d)
{
    return c + (x - a) * (d - c) / (b - a);
}

GLuint loadTexture(const QString &filePath)
{
    QImage img(filePath);
    if (img.isNull()) {
    qWarning("Failed to load texture: %s", qPrintable(filePath));
    return 0;
    }

    qInfo("Loaded texture: %s", qPrintable(filePath));

    img = img.convertToFormat(QImage::Format_RGBA8888); // ensure 4 channels

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(), 0,
         GL_RGBA, GL_UNSIGNED_BYTE, img.bits());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);
    return texID;
}

void loadShape(RenderShapeData &o)
{
    switch (o.primitive.type) {
    case PrimitiveType::PRIMITIVE_SBH: break;
    default:
        break;
    }
}

void updateShapes(RenderData &rend)
{
    int i = 0;
    for (RenderShapeData &o : rend.shapes) {
        QString texFile = QString::fromStdString(o.primitive.material.textureMap.filename);
        if (!texFile.isEmpty()) {
            rend.shapeTextures[i] = loadTexture(texFile);
            o.hasTexture = true;
        } else
            rend.shapeTextures[i] = 0;

        loadShape(o);
        i++;
    }
}

void clearShapes(RenderData &rend)
{
    rend.shapes.clear();
    rend.lights.clear();
}

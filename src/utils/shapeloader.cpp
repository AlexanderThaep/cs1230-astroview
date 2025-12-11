#include "realtime.h"
#include "settings.h"
#include "utils/sceneparser.h"

#include <iostream>

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

void updateShapes(RenderData &rend)
{
    int i = 0;
    for (auto it = rend.shapes.begin(); it != rend.shapes.end(); /* no ++ */) {
        RenderShapeData& o = *it;
        i = glm::min(i, 7);

        QString texFile = QString::fromStdString(o.primitive.material.textureMap.filename);
        if (!texFile.isEmpty()) {
            rend.shapeTextures[i] = loadTexture(texFile);
            o.hasTexture = true;
        } else {
            rend.shapeTextures[i] = 0;
            o.hasTexture = false;
        }

        if (o.primitive.type == PrimitiveType::PRIMITIVE_SBH) {
            rend.hasBH = true;
            rend.bh_pos = o.worldPos;
            rend.bh_r = glm::length(o.ctm[0]);

            it = rend.shapes.erase(it);    // returns next valid iterator
            continue;                      // skip i++ since we erased
        }

        it++;
        i++;
    }
}

void clearShapes(RenderData &rend)
{
    rend.shapes.clear();
    rend.lights.clear();

    rend.hasBH = false;
    rend.bh_pos = glm::vec3(0.0f);
    rend.bh_r = 1.0f;
}

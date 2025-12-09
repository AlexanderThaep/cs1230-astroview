#pragma once

#include <utils/sceneparser.h>

void updateShapes(RenderData &rend, bool shouldLOD);
void loadLOD(RenderShapeData &o);
void loadShape(RenderShapeData &o);
void clearShapes(RenderData &rend);

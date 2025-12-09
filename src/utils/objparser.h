#pragma once

#include "sceneparser.h"

#include <glm/glm.hpp>
#include <vector>
#include <string>

#define PARSE_INDEX(s) ((s).split('/')[0].toInt() - 1)

class OBJParser
{
public:
    struct Tri {
        glm::vec3 points[3];
        glm::vec3 normal;
        glm::vec3 centroid;
    };

    OBJParser(const RenderShapeData &obj);
    void parse(std::string meshfile, std::vector<Tri> &tris);
private:
    const RenderShapeData &parent;
};

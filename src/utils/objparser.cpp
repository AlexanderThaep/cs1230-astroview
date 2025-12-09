#include "objparser.h"

#include <QFile>
#include <vector>
#include <iostream>
#include <QTextStream>

OBJParser::OBJParser(const RenderShapeData &obj) : parent(obj) {}

void OBJParser::parse(std::string meshfile, std::vector<Tri> &tris)
{
    QFile file(meshfile.c_str());
    if (!file.open(QFile::ReadOnly)) {
        std::cout << "could not open " << meshfile << std::endl;
        return;
    }

    std::cout << "Opened " << meshfile << std::endl;

    std::vector<glm::vec3> tempVerts;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;

        if (line.startsWith("v ")) {

            QStringList parts = line.split(' ', Qt::SkipEmptyParts);
            if (parts.size() >= 4) {
                float x = parts[1].toFloat();
                float y = parts[2].toFloat();
                float z = parts[3].toFloat();
                tempVerts.push_back(glm::vec3(x, y, z));
            }

        } else if (line.startsWith("f ")) {

            QStringList parts = line.split(' ', Qt::SkipEmptyParts);
            if (parts.size() < 4) continue;

            Tri tri;

            tri.points[0] = tempVerts[PARSE_INDEX(parts[1])];
            tri.points[1] = tempVerts[PARSE_INDEX(parts[2])];
            tri.points[2] = tempVerts[PARSE_INDEX(parts[3])];

            glm::vec3 v0v1 = tri.points[1] - tri.points[0];
            glm::vec3 v0v2 = tri.points[2] - tri.points[0];

            tri.normal = glm::cross(v0v1, v0v2);
            tri.centroid = glm::vec4((tri.points[0] + tri.points[1] + tri.points[2])  / 3.0f, 1.0f);

            tris.push_back(tri);
        }
    }

    std::cout << "Parsed " << tris.size() << " triangles!" << std::endl;

    file.close();
}

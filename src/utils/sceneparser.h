#pragma once

#include "GL/glew.h"
#include "scenedata.h"
#include <vector>
#include <string>

// Struct which contains data for a single primitive, to be used for rendering
struct RenderShapeData {
    ScenePrimitive primitive;
    glm::mat4 ctm; // the cumulative transformation matrix
    glm::mat4 invCTM; // inverse ctm
    glm::mat3 invCTMT; // inverse ctm and transposed

    glm::vec3 worldPos; // World space position of the object
    int vertexCount; 	// Number of individual verts
    int param1; 		// Keep track of individual object tessellation params for optimization
    int param2;			// Second tessellation param

    GLuint vbo;
    GLuint vao;
};

// Struct which contains all the data needed to render a scene
struct RenderData {
    SceneGlobalData globalData;
    SceneCameraData cameraData;

    std::vector<SceneLightData> lights;
    std::vector<RenderShapeData> shapes;
};

class SceneParser {
public:
    // Parse the scene and store the results in renderData.
    // @param filepath    The path of the scene file to load.
    // @param renderData  On return, this will contain the metadata of the loaded scene.
    // @return            A boolean value indicating whether the parse was successful.
    static bool parse(std::string filepath, RenderData &renderData);
};

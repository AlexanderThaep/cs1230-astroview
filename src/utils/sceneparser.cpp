#include "sceneparser.h"
#include "scenefilereader.h"
#include <glm/gtx/transform.hpp>

#include <chrono>
#include <iostream>

bool SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readJSON();
    if (!success) {
        return false;
    }

    renderData.globalData = fileReader.getGlobalData();
    renderData.cameraData = fileReader.getCameraData();
    renderData.shapes.clear();
    renderData.lights.clear();
    dfs(fileReader.getRootNode(), glm::mat4(1.), renderData);
    return true;
}

void SceneParser::dfs(SceneNode* node, glm::mat4 parent_ctm, RenderData &renderData) {
    if (node == nullptr) return;

    //construct total transformation for node
    glm::mat4 node_transformations = glm::mat4(1.0f);

    for(auto transformation: node->transformations) {
        switch (transformation->type) {
        case TransformationType::TRANSFORMATION_TRANSLATE:
            node_transformations *= glm::translate(transformation->translate);
            break;

        case TransformationType::TRANSFORMATION_SCALE:
            node_transformations *= glm::scale(transformation->scale);
            break;

        case TransformationType::TRANSFORMATION_ROTATE:
            node_transformations *= glm::rotate(transformation->angle, transformation->rotate);
            break;

        case TransformationType::TRANSFORMATION_MATRIX:
            node_transformations *= transformation->matrix;
            break;

        default:
            break;
        }
    }

    //Create RenderShapeData for each primitive type
    auto current_ctm = parent_ctm * node_transformations;
    for (auto primitive: node->primitives) {
        RenderShapeData shape;
        shape.ctm = current_ctm;
        shape.primitive = *primitive;
        renderData.shapes.push_back(shape);
    }

    //Create SceneLightData for each light
    for (auto light: node->lights) {
        SceneLightData scene_light;
        scene_light.type = light->type;
        scene_light.color = light->color;
        scene_light.function = light->function;
        scene_light.pos = current_ctm * glm::vec4(0., 0., 0., 1.);
        scene_light.dir = current_ctm * light->dir;
        scene_light.penumbra = light->penumbra;
        scene_light.angle = light->angle;
        scene_light.height = light->height;
        scene_light.width = light->width;
        renderData.lights.push_back(scene_light);

    }

    //process child nodes
    for (auto child_node : node->children) {
        dfs(child_node, current_ctm, renderData);
    }


}

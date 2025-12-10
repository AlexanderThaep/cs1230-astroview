#include "sceneparser.h"
#include "scenefilereader.h"
#include <glm/gtx/transform.hpp>

#include <chrono>
#include <iostream>

void populateRenderData(glm::mat4 ctm, SceneNode *prevNode, RenderData &renderData)
{
    if (!prevNode) return;

    for (SceneTransformation *t : prevNode->transformations) {
        switch (t->type) {
        case TransformationType::TRANSFORMATION_TRANSLATE:
            ctm = glm::translate(ctm, t->translate);
            break;

        case TransformationType::TRANSFORMATION_SCALE:
            ctm = glm::scale(ctm, t->scale);
            break;

        case TransformationType::TRANSFORMATION_ROTATE:
            ctm = glm::rotate(ctm, t->angle, t->rotate);
            break;

        case TransformationType::TRANSFORMATION_MATRIX:
            ctm = ctm * t->matrix;
            break;
        }
    }

    glm::mat4 invCTM = glm::inverse(ctm);
    glm::mat3 invCTMT = glm::transpose((glm::mat3(invCTM)));

    for (SceneNode *n : prevNode->children) {
        populateRenderData(ctm, n, renderData);
    }

    int count = 0;
    for (SceneLight *l : prevNode->lights) {
        if (count > 8) break;
        glm::vec3 light_dir = glm::vec3(l->dir);
        SceneLightData d = {
            .id = l->id,
            .type = l->type,
            .color = l->color,
            .function = l->function,
            .penumbra = l->penumbra,
            .angle = l->angle,
            .width = l->width,
            .height = l->height,
        };

        d.pos = ctm * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        d.dir = glm::normalize(ctm * glm::vec4(light_dir, 0.0f));

        renderData.lights.push_back(d);
        count++;
    }

    for (ScenePrimitive *p : prevNode->primitives) {

        RenderShapeData d = {
            .primitive = *p,
            .ctm = ctm,
            .invCTM = invCTM,
            .invCTMT = invCTMT
        };

        d.worldPos = ctm * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        renderData.shapes.push_back(d);
    }
}

bool SceneParser::parse(std::string filepath, RenderData &renderData)
{
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readJSON();
    if (!success) {
        return false;
    }

    using Clock = std::chrono::high_resolution_clock;
    auto t0 = Clock::now();

    renderData.globalData = fileReader.getGlobalData();
    renderData.cameraData = fileReader.getCameraData();

    renderData.lights.clear();
    renderData.shapes.clear();

    renderData.hasBH = false;
    renderData.bh_pos = glm::vec3(0.0f);
    renderData.bh_r = 1.0f;

    populateRenderData(glm::mat4(1.0f), fileReader.getRootNode(), renderData);

    auto t1 = Clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    std::cout << "Parse time: " << ms << " ms" << std::endl;

    return true;
}

#include "scene.h"

Scene::Scene(int width, int height, const RenderData &metaData) {
    this->viewPlaneHeight = height;
    this->viewPlaneWidth = width;
    this->renderData = metaData;
    this->camera.cameraData = metaData.cameraData;
    this->camera.viewPlaneHeight = height;
    this->camera.near = 0.01;
    this->camera.far = 2.0;
    this->camera.viewPlaneWidth = width;
}

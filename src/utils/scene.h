#ifndef SCENE_H
#define SCENE_H

#include "utils/sceneparser.h"
#include "camera/camera.h"

//Holds all the scene information
class Scene
{
public:
    Scene(int viewPlaneWidth, int viewPlaneHeight, const RenderData &renderData);

    // The width of the scene
    int viewPlaneWidth;

    // The height of the scene
    int viewPlaneHeight;

    // The render data for the scene
    RenderData renderData;

    //The scene camera
    Camera camera;
};

#endif // SCENE_H

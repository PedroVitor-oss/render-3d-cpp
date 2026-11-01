#ifndef ENTITY_H
#define ENTITY_H
#include "raylib.h"
#include "model3d.h"
#include "renderqueue.h"
#include "camera.h"
#include "engine_types.h"

class Entity
{
private:
    Model3D model;
    Model3D saveModel;

public:
    Vector3 position = {0, 0, 0};
    Vector3 rotation = {0, 0, 0};
    Vector3 scale = {1, 1, 1};
    int layer = LAYER_OBJECTS;

    void SetScale(Vector3 s);

    void SetModel(Model3D m);
    void Render(enginep::Camera &cam, RenderQueue &queue);
};

#endif
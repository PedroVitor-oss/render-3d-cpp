#include "raylib.h"
#include "rlgl.h"
#include <vector>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <map>

#include "camera.h"
#include "math3D.h"
#include "light.h"
#include "renderqueue.h"
#include "model3d.h"
#include "entity.h"


int main()
{
    // carregando modelos
    Model3D mokeyModel;
    Model3D plane;

    mokeyModel.Init("models/monkey.obj","models/monkey.mtl");
    // plane.Init("models/plane.mdl");

    Model3D floorModel;
    floorModel.BuildGrid(20, 10.0f, DARKGRAY, GRAY);

    // criando entidade
    Entity monkey;
    monkey.SetModel(mokeyModel);
    

    Entity floor;
    floor.SetModel(floorModel);
    floor.position.y = -10;
    floor.layer = LAYER_FLOOR;

    GlobalLight light({0.5f, 1, 0.5f}, WHITE, 1.0f);

    enginep::Camera cam;
    cam.camPos.z = -50;

    RenderQueue queue;
    queue.SetLight(&light);

    monkey.SetScale({10, 10, 10});

    InitWindow(WIDTH_WINDOWS, HEIGHT_WINDOWS, "engine 3d");
    SetTargetFPS(60);

    rlDisableBackfaceCulling();
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        cam.MoveCam(dt);
        
        if (IsKeyPressed(KEY_TAB))
            queue.wireframe = !queue.wireframe;

        BeginDrawing();
        ClearBackground(BLACK);
        floor.Render(cam, queue);
        monkey.Render(cam, queue);
        queue.Flush();

        EndDrawing();
    }
    CloseWindow();
    return 0;
}


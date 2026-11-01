#ifndef RENDERQUEUE_H
#define RENDERQUEUE_H
#include "raylib.h"
#include "light.h"
#include <vector>
#include <iostream>
#include <algorithm>



struct ScreenTri
{
    Vector2 p[3];
    float depth;
    int layer;
    Color color;
};

class RenderQueue
{
private:
    std::vector<ScreenTri> tris;

public:
    bool wireframe = true;
    GlobalLight *light;

    void SetLight(GlobalLight *l);

    void Push(const ScreenTri &t);

    void Flush();
};


#endif
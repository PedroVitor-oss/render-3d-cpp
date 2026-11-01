#ifndef FACE_H
#define FACE_h
#include "raylib.h"
#include "engine_types.h"
#include "renderqueue.h"
#include "light.h"
#include "math3d.h"
#include "camera.h"

#include <vector>
#include <iostream>

class Face
{

    int a, b, c;
    Color cl = WHITE;

public:
    Face(int a, int b, int c, Color color = WHITE)
        : a(a), b(b), c(c), cl(color) {}

    Vector3 NormalFace(const std::vector<Vector3> &vertices) const;

    bool isFrontFacing(const std::vector<Vector3> &vertices, const enginep::Camera &cam) const;
    void Submit(const std::vector<Vector3> &vertices, const enginep::Camera &cam,
                RenderQueue &queue, int layer) const;
    void draw(const std::vector<Vector3> &vertices, enginep::Camera &cam) const;

    void drawWireframe(const std::vector<Vector3> &vertices, enginep::Camera &cam) const;
};

#endif
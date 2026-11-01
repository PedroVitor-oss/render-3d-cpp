#ifndef MATH3D_H
#define MATH3D_H
#include "raylib.h"
#include <math.h>
#include "camera.h"
constexpr int WIDTH_WINDOWS = 700;
constexpr int HEIGHT_WINDOWS = 350;
constexpr float FOV = 60.0f;
constexpr float NEAR_PLANE = 0.1;


Vector3 GetForwardVector(Vector3 rotate);

Vector3 GetRightVector(Vector3 rotate);

Vector3 WorldCam(Vector3 pointInWorld, const enginep::Camera &cam);
Vector2 PointPerspective(Vector3 v);
Vector2 WorldToScreen(Vector3 v, const enginep::Camera &cam);

bool DrawLine3D(Vector3 a, Vector3 b, enginep::Camera &cam, Color color);

int ClipNear(const Vector3 in[3], Vector3 out[4]);

#endif
#ifndef CAMERA_H
#define CAMERA_H
#include "raylib.h"

namespace enginep
{
    class Camera
    {
    private:
        float speed = 10;
        float speedRotate = 1.0f;
        float vy = 0;

    public:
        Vector3 camPos = {0, 0, 0};
        Vector3 rotate = {0, 0, 0};
        void MoveCam(float dt);
    };
};

#endif
#include "raylib.h"
#include "camera.h"
#include "math3D.h"

void enginep::Camera::MoveCam(float dt)
{

    Vector3 forward = GetForwardVector(this->rotate);
    Vector3 right = GetRightVector(this->rotate);

    if (IsKeyDown(KEY_W))
    {
        this->camPos.z += this->speed * dt * forward.z;
        this->camPos.x -= this->speed * dt * forward.x;
    }
    if (IsKeyDown(KEY_S))
    {
        this->camPos.z -= this->speed * dt * forward.z;
        this->camPos.x += this->speed * dt * forward.x;
    }
    if (IsKeyDown(KEY_A))
    {
        this->camPos.z += this->speed * dt * right.z;
        this->camPos.x -= this->speed * dt * right.x;
    }
    if (IsKeyDown(KEY_D))
    {
        this->camPos.z -= this->speed * dt * right.z;
        this->camPos.x += this->speed * dt * right.x;
    }

    if (IsKeyDown(KEY_LEFT))
        this->rotate.y += this->speedRotate * dt;
    if (IsKeyDown(KEY_RIGHT))
        this->rotate.y -= this->speedRotate * dt;
    if (IsKeyDown(KEY_UP))
        this->rotate.x -= this->speedRotate * dt;
    if (IsKeyDown(KEY_DOWN))
        this->rotate.x += this->speedRotate * dt;

    if (IsKeyPressed(KEY_SPACE))
    {
        this->camPos.y += 0.5f;
        this->vy = 1;
    }
    this->vy -= 0.9f * dt;

    if (this->camPos.y <= 0)
    {
        this->vy = 0;
        this->camPos.y = 0;
    }
    this->camPos.y += this->vy;
}
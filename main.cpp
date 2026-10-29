#include "raylib.h"
#include <vector>
#include <math.h>

constexpr int WIDTH_WINDOWS = 700;
constexpr int HEIGHT_WINDOWS = 350;
constexpr float WORLD_SCALE = 10.0f;
// cofiguraçaõ de camera
constexpr float FOV = 60.0f;
constexpr float znear = 0.1f;
constexpr float zfar = 50.0f;
namespace enginep
{
    class Camera
    {
    public:
        Vector3 camPos = {0, 0, 0};
    };
};
Vector2 PointPerspective(Vector3 v, enginep::Camera cam)
{
    float aspect = (float)WIDTH_WINDOWS / (float)HEIGHT_WINDOWS;
    float angleRAD = FOV * (PI / 180);
    float f = 1 / tan(angleRAD / 2);

    Vector2 v2;
    if (v.z <= 0.0001f)
        return {-10000, -10000};

    // mnultiplixações bizaras
    v2.x = ((v.x + cam.camPos.x )/ (v.z + cam.camPos.z)) * (f / aspect) * (WIDTH_WINDOWS / 2) + (WIDTH_WINDOWS / 2);
    v2.y = (-v.y / (v.z + cam.camPos.z)) * (f) * (HEIGHT_WINDOWS / 2) + (HEIGHT_WINDOWS / 2);

    return v2;
}

Vector2 WorldToScreen(Vector2 v)
{
    // Y invertido: +Y mundo = pra cima na tela
    return {v.x * WORLD_SCALE + WIDTH_WINDOWS * 0.5f,
            -v.y * WORLD_SCALE + HEIGHT_WINDOWS * 0.5f};
}

// class Triangle
// {
//     Vector3 a, b, c;

// public:
//     Triangle(Vector3 a, Vector3 b, Vector3 c) : a(a), b(b), c(c) {}

//     void draw() const
//     {
//         // DrawTriangle(PointPerspective(a), PointPerspective(b), PointPerspective(c), WHITE);
//     }
// };

class Face
{
    int a, b, c;

public:
    Face(int a, int b, int c) : a(a), b(b), c(c) {}
    void draw(const std::vector<Vector3> &vertices, enginep::Camera &cam) const
    {
        DrawTriangle(PointPerspective(vertices[this->a], cam),
                     PointPerspective(vertices[this->b], cam),
                     PointPerspective(vertices[this->c], cam),
                     WHITE);
    }
};

int main()
{
    InitWindow(WIDTH_WINDOWS, HEIGHT_WINDOWS, "engine 3d");
    SetTargetFPS(60);
    // vertices de um plano
    std::vector<Vector3> vertices = {
        {10, 10, 30},
        {-10, 10, 30},
        {-10, -10, 30},
        {10, -10, 30}};
    // faces do plano
    std::vector<Face> faces = {
        {0, 1, 3},
        {1, 2, 3}};

    enginep::Camera cam;
    while (!WindowShouldClose())
    {

        float dt = GetFrameTime();
        float speed = 10;
        if (IsKeyDown(KEY_A))
            cam.camPos.x += speed * dt;
        if (IsKeyDown(KEY_D))
            cam.camPos.x -= speed * dt;
        if (IsKeyDown(KEY_W))
            cam.camPos.z -= speed * dt;
        if (IsKeyDown(KEY_S))
            cam.camPos.z += speed * dt;

        BeginDrawing();
        ClearBackground(BLACK);
        for (const auto &f : faces)
        {
            f.draw(vertices, cam);
        }

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
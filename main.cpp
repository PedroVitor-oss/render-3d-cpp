#include "raylib.h"
#include <vector>
#include <math.h>
#include <algorithm>

constexpr int WIDTH_WINDOWS = 700;
constexpr int HEIGHT_WINDOWS = 350;
constexpr float FOV = 60.0f;

namespace enginep
{
    class Camera
    {
    public:
        Vector3 camPos = {0, 0, 0};
    };
};

Vector3 WorldCam(Vector3 pointInWorld, const enginep::Camera &cam)
{
    return {
        pointInWorld.x - cam.camPos.x,
        pointInWorld.y - cam.camPos.y,
        pointInWorld.z - cam.camPos.z};
}

Vector2 PointPerspective(Vector3 v)
{
    float aspect = (float)WIDTH_WINDOWS / (float)HEIGHT_WINDOWS;
    float angleRAD = FOV * (PI / 180);
    float f = 1 / tan(angleRAD / 2);

    Vector2 v2;
    if (v.z <= 0.0001f)
        return {-10000, -10000};

    v2.x = (v.x / v.z) * (f / aspect) * (WIDTH_WINDOWS / 2) + (WIDTH_WINDOWS / 2);
    v2.y = (-v.y / v.z) * (f) * (HEIGHT_WINDOWS / 2) + (HEIGHT_WINDOWS / 2);

    return v2;
}

Vector2 WorldToScreen(Vector3 v, const enginep::Camera &cam)
{
    return PointPerspective(WorldCam(v, cam));
}

class Face
{
    int a, b, c;
    Color cl = WHITE;

public:
    Face(int a, int b, int c, Color color = WHITE)
        : a(a), b(b), c(c), cl(color) {}

    // NOVO: Verifica se a face está virada para a câmera
    bool isFrontFacing(const std::vector<Vector3> &vertices, const enginep::Camera &cam) const
    {
        Vector3 A = vertices[a];
        Vector3 B = vertices[b];
        Vector3 C = vertices[c];

        // 1. Calcula os vetores da face
        Vector3 AB = {B.x - A.x, B.y - A.y, B.z - A.z};
        Vector3 AC = {C.x - A.x, C.y - A.y, C.z - A.z};

        // 2. Produto vetorial para achar a Normal da face
        Vector3 normal = {
            AB.y * AC.z - AB.z * AC.y,
            AB.z * AC.x - AB.x * AC.z,
            AB.x * AC.y - AB.y * AC.x};

        // 3. Vetor da face até a câmera
        Vector3 toCam = {cam.camPos.x - A.x, cam.camPos.y - A.y, cam.camPos.z - A.z};

        // 4. Produto escalar (Dot Product). Se for > 0, a normal aponta para a câmera.
        float dot = normal.x * toCam.x + normal.y * toCam.y + normal.z * toCam.z;
        return dot > 0;
    }

    void draw(const std::vector<Vector3> &vertices, enginep::Camera &cam) const
    {
        if (!isFrontFacing(vertices, cam))
            return;

        DrawTriangle(WorldToScreen(vertices[this->a], cam),
                     WorldToScreen(vertices[this->c], cam), // c antes de b
                     WorldToScreen(vertices[this->b], cam),
                     cl);
    }
};

int main()
{
    InitWindow(WIDTH_WINDOWS, HEIGHT_WINDOWS, "engine 3d");
    SetTargetFPS(60);

    std::vector<Vector3> vertices = {
        {10, 10, 30},   // 0
        {-10, 10, 30},  // 1
        {-10, -10, 30}, // 2
        {10, -10, 30},  // 3
        {10, 10, 50},   // 4
        {-10, 10, 50},  // 5
        {-10, -10, 50}, // 6
        {10, -10, 50}}; // 7

    std::vector<Face> faces = {
        // frente
        {0, 3, 1, RED},
        {1, 3, 2, RED},
        // trás
        {4, 5, 7, BLUE},
        {5, 6, 7, BLUE},
        // lateral direita
        {0, 4, 3, GREEN},
        {4, 7, 3, GREEN},
        // lateral esquerda
        {1, 2, 5, YELLOW},
        {5, 2, 6, YELLOW},
        // topo
        {0, 5, 4, ORANGE},
        {0, 1, 5, ORANGE},
        // base
        {3, 7, 6, PURPLE},
        {3, 6, 2, PURPLE}};

    enginep::Camera cam;

    float vycam = 0;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        float speed = 10;
        if (IsKeyDown(KEY_A))
            cam.camPos.x -= speed * dt;
        if (IsKeyDown(KEY_D))
            cam.camPos.x += speed * dt;
        if (IsKeyDown(KEY_W))
            cam.camPos.z += speed * dt;
        if (IsKeyDown(KEY_S))
            cam.camPos.z -= speed * dt;

        // pequena fisic de pulo para ve a face de cima 
        if(IsKeyPressed(KEY_SPACE)){
            cam.camPos.y+=0.5f;
             vycam = 1;
        }
        vycam -= 0.9f * dt;

        if(cam.camPos.y<=0){
            vycam =0;
            cam.camPos.y =0;
        }
         cam.camPos.y += vycam;


        BeginDrawing();
        ClearBackground(BLACK);

        // SEM SORT! O Backface Culling já resolve o problema para este cubo.
        for (const auto &f : faces)
            f.draw(vertices, cam);

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
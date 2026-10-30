#include "raylib.h"
#include <vector>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

constexpr int WIDTH_WINDOWS = 700;
constexpr int HEIGHT_WINDOWS = 350;
constexpr float FOV = 60.0f;

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
        Vector3 rotate = {0, 0, 0}; // 0, eixo y , 0
        void MoveCam(float dt);
    };
};

Vector3 GetForwardVector(Vector3 rotate)
{
    return {
        sinf(rotate.y) * cosf(rotate.x),
        -sinf(rotate.x),
        cosf(rotate.y) * cosf(rotate.x)};
}

Vector3 GetRightVector(Vector3 rotate)
{
    return {
        cosf(rotate.y) * cosf(rotate.z) + sinf(rotate.y) * sinf(rotate.x) * sinf(rotate.z),
        cosf(rotate.x) * sinf(rotate.z),
        -sin(rotate.y) * cosf(rotate.z) + cosf(rotate.y) * sinf(rotate.x) * sinf(rotate.z)};
}

Vector3 WorldCam(Vector3 pointInWorld, const enginep::Camera &cam)
{
    Vector3 p = {
        pointInWorld.x - cam.camPos.x,
        pointInWorld.y - cam.camPos.y,
        pointInWorld.z - cam.camPos.z};

    float cy = cosf(-cam.rotate.y);
    float sy = sinf(-cam.rotate.y);

    float cx = cosf(-cam.rotate.x);
    float sx = sinf(-cam.rotate.x);

    float tempx = p.x;
    p.x = p.x * cy - p.z * sy;
    p.z = tempx * sy + p.z * cy;

    float tempy = p.y;
    p.y = p.y * cx - p.z * sx;
    p.z = tempy * sx + p.z * cx;
    return p;
}
Vector2 PointPerspective(Vector3 v)
{
    float aspect = (float)WIDTH_WINDOWS / (float)HEIGHT_WINDOWS;
    float angleRAD = FOV * (PI / 180);
    float f = 1 / tan(angleRAD / 2);

    Vector2 v2;
    if (v.z <= 0.000001f)
        return {-10000, -10000};

    v2.x = (v.x / v.z) * (f / aspect) * (WIDTH_WINDOWS / 2) + (WIDTH_WINDOWS / 2);
    v2.y = (-v.y / v.z) * (f) * (HEIGHT_WINDOWS / 2) + (HEIGHT_WINDOWS / 2);

    return v2;
}

Vector2 WorldToScreen(Vector3 v, const enginep::Camera &cam)
{
    return PointPerspective(WorldCam(v, cam));
}

// Retorna true se a linha foi desenhada com sucesso
bool DrawLine3D(Vector3 a, Vector3 b, enginep::Camera &cam, Color color)
{
    // 1. Transforma para o espaço da câmera
    Vector3 ca = WorldCam(a, cam);
    Vector3 cb = WorldCam(b, cam);

    // 2. Se um dos pontos estiver atrás da câmera, ignora a linha inteira
    // (Isso evita o efeito de linhas cruzando a tela)
    if (ca.z <= 0.1f || cb.z <= 0.1f)
        return false;

    // 3. Projeta para a tela
    Vector2 pa = PointPerspective(ca);
    Vector2 pb = PointPerspective(cb);

    // 4. Desenha
    DrawLineV(pa, pb, color);
    return true;
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
        this->drawWireframe(vertices, cam);
    }
    // Adicione dentro da classe Face, no public:
    void drawWireframe(const std::vector<Vector3> &vertices, enginep::Camera &cam) const
    {
        if (!isFrontFacing(vertices, cam))
            return;

        Vector2 pA = WorldToScreen(vertices[this->a], cam);
        Vector2 pB = WorldToScreen(vertices[this->b], cam);
        Vector2 pC = WorldToScreen(vertices[this->c], cam);

        // Desenha as 3 arestas do triângulo em preto
        DrawLineV(pA, pB, BLACK);
        DrawLineV(pB, pC, BLACK);
        DrawLineV(pC, pA, BLACK);

        // Desenha os pontinhos nos vértices (opcional, mas fica bonitinho)
        // DrawCircleV(pA, 3, RED);
        // DrawCircleV(pB, 3, RED);
        // DrawCircleV(pC, 3, RED);
    }
};

class Model3D
{
private:
    std::vector<Face> faces;
    void AddVertice(std::string data)
    {
        std::stringstream stream(data);

        char type;
        Vector3 v;
        stream >> type >> v.x >> v.y >> v.z;
        // std::cout<<"AddVertice - id: "<<vertices.size()<<std::endl;
        vertices.push_back(v);
    }
    void AddFace(std::string data)
    {
        std::stringstream stream(data);
        char type;
        int A, B, C, COLORID;
        stream >> type >> A >> B >> C >> COLORID;
        Face f = {A, B, C, WHITE};
        faces.push_back(f);
    }

public:
    std::vector<Vector3> vertices;
    void Init(std::string path)
    {
        // abrir arquivo
        std::fstream file;
        file.open(path);
        if (!file.is_open())
        {
            std::cout << "Erro ao abrir arquivo do modelo 3d. Path file: " << path << std::endl;
            file.close();
            return;
        }

        // std::cout<<"arquivo aberto com sucesso";
        std::string line;
        while (std::getline(file, line))
        {
            std::cout << line << std::endl;
            switch ((char)line[0])
            {
            case 'v':
                this->AddVertice(line);
                break;
            case 'f':
                this->AddFace(line);
                break;
            default:
                break;
            }
        }

        file.close();
    }
    void Renderer(enginep::Camera &cam)
    {
        for (const auto &f : this->faces)
            f.draw(vertices, cam);
    }
};

class Entity
{
private:
    Model3D model;
    Model3D saveModel;

public:
    Vector3 position = {0,0,0};
    Vector3 rotation = {0,0,0};
    void SetModel(Model3D m)
    {
        this->model = m;
        this->saveModel = m;
    }
    void Render(enginep::Camera &cam)
    {
        // para cada vertice adiciaonr um offset que no caso seria o position
        // for (int i =0 ; i < this->saveModel.vertices.size(); i++){
        //     Vector3 nv = {
        //         this->saveModel.vertices[i].x + this->position.x,
        //         this->saveModel.vertices[i].y + this->position.y,
        //         this->saveModel.vertices[i].z + this->position.z,
        //     };
        //     this->model.vertices[i] = nv;
        // }
        // ROTACIONAR O objeto 
        for (int i =0 ; i < this->saveModel.vertices.size(); i++){
            float x = this->saveModel.vertices[i].x;
            float z = this->saveModel.vertices[i].z;
            
            float nx = (cosf(this->rotation.y) * x) - (z * sinf(this->rotation.y));
            float ny = (sinf(this->rotation.y) * x) + (z * cosf(this->rotation.y));
            Vector3 nv = {
                nx + this->position.x,
                this->saveModel.vertices[i].y + this->position.y,
                ny + this->position.z,
            };
            this->model.vertices[i] = nv;
        }

        this->model.Renderer(cam);
    }
};

int main()
{
    // carregar modelos
    Model3D cuboModel;
    Model3D plane;

    cuboModel.Init("models/cubo.mdl");
    plane.Init("models/plane.mdl");

    // criando entidades
    std::vector<Entity> entidades;
    for(int i =1; i <= 1; i ++){
        Entity cubo;
        cubo.SetModel(cuboModel);
        cubo.position.x = -20 + (20 * i);
        entidades.push_back(cubo);
    }

    // return 1;
    InitWindow(WIDTH_WINDOWS, HEIGHT_WINDOWS, "engine 3d");
    SetTargetFPS(60);

    enginep::Camera cam;
    cam.camPos.z = -50;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        cam.MoveCam(dt);
        if(IsKeyDown(KEY_P)){
            entidades[0].rotation.y += 1 * dt ;
        }
        // if(IsKeyDown(KEY_KP_6)){
        //     cubo.position.x += 3 * dt ;
        // }

        BeginDrawing();
        ClearBackground(BLACK);

        for(auto& cubo : entidades){
            cubo.Render(cam);
        }
        // plane.Renderer(cam);
        // for (const auto &f : faces)
        //     f.draw(vertices, cam);

        EndDrawing();
    }
    CloseWindow();
    return 0;
}

void enginep::Camera::MoveCam(float dt)
{

    // movimento

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

    // rotação
    if (IsKeyDown(KEY_LEFT))
        this->rotate.y += this->speedRotate * dt;
    if (IsKeyDown(KEY_RIGHT))
        this->rotate.y -= this->speedRotate * dt;
    if (IsKeyDown(KEY_UP))
        this->rotate.x -= this->speedRotate * dt;
    if (IsKeyDown(KEY_DOWN))
        this->rotate.x += this->speedRotate * dt;

    // pequena fisic de pulo para ve a face de cima
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
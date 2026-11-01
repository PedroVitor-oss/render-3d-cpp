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

constexpr int WIDTH_WINDOWS = 700;
constexpr int HEIGHT_WINDOWS = 350;
constexpr float FOV = 60.0f;
constexpr float NEAR_PLANE = 0.1;
enum Layer
{
    LAYER_FLOOR = 0,
    LAYER_OBJECTS
};

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

bool DrawLine3D(Vector3 a, Vector3 b, enginep::Camera &cam, Color color)
{

    Vector3 ca = WorldCam(a, cam);
    Vector3 cb = WorldCam(b, cam);

    if (ca.z <= 0.1f || cb.z <= 0.1f)
        return false;

    Vector2 pa = PointPerspective(ca);
    Vector2 pb = PointPerspective(cb);

    DrawLineV(pa, pb, color);
    return true;
}

int ClipNear(const Vector3 in[3], Vector3 out[4])
{
    int n = 0;
    for (int i = 0; i < 3; i++)
    {
        Vector3 cur = in[i];
        Vector3 nxt = in[(i + 1) % 3];
        bool curIn = cur.z >= NEAR_PLANE;
        bool nxtIn = nxt.z >= NEAR_PLANE;

        if (curIn)
            out[n++] = cur;

        if (curIn != nxtIn)
        {
            float t = (NEAR_PLANE - cur.z) / (nxt.z - cur.z);
            out[n++] = {
                cur.x + (nxt.x - cur.x) * t,
                cur.y + (nxt.y - cur.y) * t,
                NEAR_PLANE};
        }
    }
    return n;
}
struct ScreenTri
{
    Vector2 p[3];
    float depth;
    int layer;
    Color color;
};
class GlobalLight
{
    Vector3 dir;
    Color cl;
    float intensity;

public:
    GlobalLight(Vector3 lightdir, Color cl, float intenxity)
    {
        // normalização de vetor direção
        float mag = std::sqrt(lightdir.x * lightdir.x + lightdir.y * lightdir.y + lightdir.z * lightdir.z);
        this->dir = {
            lightdir.x / mag,
            lightdir.y / mag,
            lightdir.z / mag};
        this->cl = cl;
        this->intensity = intenxity;
    }
    float GetIntensity()
    {
        return this->intensity;
    }
    Vector3 GetDir() const
    {
        return this->dir;
    }
};

class RenderQueue
{
private:
    std::vector<ScreenTri> tris;

public:
    bool wireframe = true;
    GlobalLight *light;

    void SetLight(GlobalLight *l)
    {
        this->light = l;
    }

    void Push(const ScreenTri &t) { tris.push_back(t); }

    void Flush()
    {
        std::sort(tris.begin(), tris.end(), [](const ScreenTri &a, const ScreenTri &b)
                  {
            if (a.layer != b.layer)
                return a.layer < b.layer;  
            return a.depth > b.depth; });

        for (const auto &t : tris)
        {
            DrawTriangle(t.p[0], t.p[1], t.p[2], t.color);
            if (wireframe)
            {
                DrawLineV(t.p[0], t.p[1], BLACK);
                DrawLineV(t.p[1], t.p[2], BLACK);
                DrawLineV(t.p[2], t.p[0], BLACK);
            }
        }
        tris.clear();
    }
};

class Face
{

    int a, b, c;
    Color cl = WHITE;

public:
    Face(int a, int b, int c, Color color = WHITE)
        : a(a), b(b), c(c), cl(color) {}

    Vector3 NormalFace(const std::vector<Vector3> &vertices) const
    {
        Vector3 A = vertices[a];
        Vector3 B = vertices[b];
        Vector3 C = vertices[c];

        Vector3 AB = {B.x - A.x, B.y - A.y, B.z - A.z};
        Vector3 AC = {C.x - A.x, C.y - A.y, C.z - A.z};

        Vector3 normal = {
            AB.y * AC.z - AB.z * AC.y,
            AB.z * AC.x - AB.x * AC.z,
            AB.x * AC.y - AB.y * AC.x};

        // float dot = normal.x * A.x + normal.y * A.y + normal.z * A.z;
        // if (dot < 0)
        // {

        //     normal.x = -normal.x;
        //     normal.y = -normal.y;
        //     normal.z = -normal.z;
        // }
        // if (fabs(normal.y) > fabs(normal.x) && fabs(normal.y) > fabs(normal.z))
        // {
        //     normal.y = fabs(normal.y);
        // }
        float length = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
        if (length > 0.0001f)
        {
            normal.x /= length;
            normal.y /= length;
            normal.z /= length;
        }

        return normal;
    }

    bool isFrontFacing(const std::vector<Vector3> &vertices, const enginep::Camera &cam) const
    {
        Vector3 A = vertices[a];
        Vector3 B = vertices[b];
        Vector3 C = vertices[c];

        Vector3 normal = NormalFace(vertices);

        Vector3 toCam = {cam.camPos.x - A.x, cam.camPos.y - A.y, cam.camPos.z - A.z};

        float dot = normal.x * toCam.x + normal.y * toCam.y + normal.z * toCam.z;
        return dot > 0;
    }
    void Submit(const std::vector<Vector3> &vertices, const enginep::Camera &cam,
                RenderQueue &queue, int layer) const
    {
        if (!isFrontFacing(vertices, cam))
            return;

        Vector3 v[3] = {
            WorldCam(vertices[a], cam),
            WorldCam(vertices[b], cam),
            WorldCam(vertices[c], cam)};

        Vector3 poly[4];
        int n = ClipNear(v, poly);
        if (n < 3)
            return;

        // clacular nova cor com base na luz globa dot product
        Vector3 normal = NormalFace(vertices);
        Vector3 lightDir = queue.light->GetDir(); // light->GetDir();
        float shine = (normal.x * lightDir.x) + (normal.y * lightDir.y) + (normal.z * lightDir.z);
        shine = std::max(0.0f, shine);
        // shine = std::min(1.0f, shine);
        float finalShine = shine * 0.7 + 0.3;
        if (layer == LAYER_FLOOR)
        {
            finalShine = 1.0f;
        }
        Color ncl;
        ncl.r = (unsigned char)(cl.r * finalShine);
        ncl.g = (unsigned char)(cl.g * finalShine);
        ncl.b = (unsigned char)(cl.b * finalShine);
        ncl.a = 255;

        for (int i = 1; i < n - 1; i++)
        {
            ScreenTri t;
            t.p[0] = PointPerspective(poly[0]);
            t.p[1] = PointPerspective(poly[i]);
            t.p[2] = PointPerspective(poly[i + 1]);
            t.depth = (poly[0].z + poly[i].z + poly[i + 1].z) / 3.0f;
            t.layer = layer;
            t.color = ncl;
            queue.Push(t);
        }
    }
    void draw(const std::vector<Vector3> &vertices, enginep::Camera &cam) const
    {
        if (!isFrontFacing(vertices, cam))
            return;

        DrawTriangle(WorldToScreen(vertices[this->a], cam),
                     WorldToScreen(vertices[this->c], cam),
                     WorldToScreen(vertices[this->b], cam),
                     cl);
        this->drawWireframe(vertices, cam);
    }

    void drawWireframe(const std::vector<Vector3> &vertices, enginep::Camera &cam) const
    {
        if (!isFrontFacing(vertices, cam))
            return;

        Vector2 pA = WorldToScreen(vertices[this->a], cam);
        Vector2 pB = WorldToScreen(vertices[this->b], cam);
        Vector2 pC = WorldToScreen(vertices[this->c], cam);

        DrawLineV(pA, pB, BLACK);
        DrawLineV(pB, pC, BLACK);
        DrawLineV(pC, pA, BLACK);
    }
};

class Model3D
{
private:
    std::vector<Face> faces;
    std::map<std::string, Color> textureMap;
    void AddVertice(std::string data)
    {
        std::stringstream stream(data);

        char type;
        Vector3 v;
        stream >> type >> v.x >> v.y >> v.z;

        vertices.push_back(v);
    }
    void AddFace(std::string data, Color color)
    {
        std::stringstream stream(data);
        char type;
        int A, B, C, COLORID;
        stream >> type;
        // modelo para .obj é f id/M/N(chamar de token)  id/M/N  id/M/N
        std::string token;
        std::vector<int> ids;
        while (stream >> token)
        {
            // pegar o id da primeira /
            int pos = token.find('/');
            std::string idStr = token.substr(0, pos);
            int id = std::stoi(idStr) - 1;
            ids.push_back(id);
        }
        A = ids[0];
        B = ids[1];
        C = ids[2];

        Face f = {A, B, C, color};
        faces.push_back(f);
    }
    void ReadModelOBJ(std::string path)
    {
        std::fstream file;
        file.open(path);
        if (!file.is_open())
        {
            std::cout << "Erro ao abrir arquivo do modelo 3d. Path file: " << path << std::endl;
            file.close();
            return;
        }

        std::string line;
        Color currentColor = WHITE;
        while (std::getline(file, line))
        {
            // std::cout << line << std::endl;
            int div = line.find(" ");
            std::string prefix = line.substr(0, div);
            if(prefix == "usemtl"){
                std::string colorName = line.substr(div+1);
                if(textureMap.find(colorName) != textureMap.end()){
                    currentColor = textureMap[colorName];
                }
            }else if (prefix == "v")
            {
                this->AddVertice(line);
            }
            else if (prefix == "f")
            {
                this->AddFace(line,currentColor);
            }
        }

        file.close();
    }
    void ReadTextureFile(std::string path)
    {
     std::fstream file;
     file.open(path);
     if(!file.is_open()){
        TraceLog(LOG_WARNING, "Erro ao abrir arquivo de textura, path: %s",path);
        file.close();
        return;
     }
     std::string line;
     std::string colorName;
     Color colorValue;
     bool seachColorValue = false;
     while(std::getline(file,line)){
        int div = line.find(" ");
        std::string prefix = line.substr(0,div);
        if(prefix == "newmtl"){
            colorName = line.substr(div+1);
            seachColorValue = true;

        }
        if(prefix == "Kd" && seachColorValue){
            std::stringstream stream(line.substr(div+1));
            float r,g,b;
            stream >>r >>g >>b;
            colorValue.r = (unsigned char)(r*255);
            colorValue.g = (unsigned char)(g*255);
            colorValue.b = (unsigned char)(b*255);
            colorValue.a = 255;
            textureMap[colorName] = colorValue;
            seachColorValue = false;
        }

     }
    }
public:
    std::vector<Vector3> vertices;
    void AddTriangle(int a, int b, int c, Color color)
    {
        faces.push_back(Face(a, b, c, color));
    }
    void BuildGrid(int tiles, float tileSize, Color c1, Color c2)
    {
        float half = tiles * tileSize * 0.5f;
        for (int j = 0; j <= tiles; j++)
            for (int i = 0; i <= tiles; i++)
                vertices.push_back({-half + i * tileSize, 0.0f, -half + j * tileSize});

        auto idx = [&](int i, int j)
        { return j * (tiles + 1) + i; };

        for (int j = 0; j < tiles; j++)
        {
            for (int i = 0; i < tiles; i++)
            {
                Color c = ((i + j) % 2 == 0) ? c1 : c2;

                AddTriangle(idx(i, j), idx(i, j + 1), idx(i + 1, j), c);
                AddTriangle(idx(i + 1, j), idx(i, j + 1), idx(i + 1, j + 1), c);
            }
        }
    }
    void Init(std::string pathModels, std::string pathTexture = "")
    {

        if (!pathTexture.empty())
        {
            this->ReadTextureFile(pathTexture);
        }
        this->ReadModelOBJ(pathModels);
    }

    void Renderer(enginep::Camera &cam)
    {
        for (const auto &f : this->faces)
            f.draw(vertices, cam);
    }
    void Submit(const enginep::Camera &cam, RenderQueue &queue, int layer)
    {
        for (const auto &f : this->faces)
            f.Submit(vertices, cam, queue, layer);
    }
};

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

    void SetScale(Vector3 s)
    {

        this->scale = s;
    }

    void SetModel(Model3D m)
    {
        this->model = m;
        this->saveModel = m;
    }
    void Render(enginep::Camera &cam, RenderQueue &queue)
    {

        for (int i = 0; i < this->saveModel.vertices.size(); i++)
        {
            float x = this->saveModel.vertices[i].x * this->scale.x;
            float y = this->saveModel.vertices[i].y * this->scale.y;
            float z = this->saveModel.vertices[i].z * this->scale.z;

            // Aplica a rotação Y
            float nx = (cosf(this->rotation.y) * x) - (z * sinf(this->rotation.y));
            float ny = (sinf(this->rotation.y) * x) + (z * cosf(this->rotation.y));

            Vector3 nv = {
                nx + this->position.x,
                y + this->position.y,
                ny + this->position.z,
            };
            this->model.vertices[i] = nv;
        }

        this->model.Submit(cam, queue, this->layer);
    }
};

int main()
{
    // carregando modelos
    Model3D cuboModel;
    Model3D plane;

    cuboModel.Init("models/casa.obj","models/casa.mtl");
    // plane.Init("models/plane.mdl");

    Model3D floorModel;
    floorModel.BuildGrid(20, 10.0f, DARKGRAY, GRAY);

    // criando entidade
    std::vector<Entity> entidades;
    for (int i = 2; i < 3; i++)
    {
        Entity cubo;
        cubo.SetModel(cuboModel);
        cubo.position.x = -30 + (30 * i);
        cubo.layer = LAYER_OBJECTS;
        entidades.push_back(cubo);
    }

    Entity floor;
    floor.SetModel(floorModel);
    floor.position.y = -10;
    floor.layer = LAYER_FLOOR;

    GlobalLight light({0.5f, 1, 0.5f}, WHITE, 1.0f);

    enginep::Camera cam;
    cam.camPos.z = -50;

    RenderQueue queue;
    queue.SetLight(&light);

    entidades[0].SetScale({10, 10, 10});

    InitWindow(WIDTH_WINDOWS, HEIGHT_WINDOWS, "engine 3d");
    SetTargetFPS(60);

    rlDisableBackfaceCulling();
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        cam.MoveCam(dt);

        if (IsKeyDown(KEY_P))
        {
            entidades[0].rotation.y += 1 * dt;
        }
        if (IsKeyPressed(KEY_TAB))
            queue.wireframe = !queue.wireframe;

        BeginDrawing();
        ClearBackground(BLACK);
        floor.Render(cam, queue);
        for (auto &cubo : entidades)
        {
            cubo.Render(cam, queue);
        }

        queue.Flush();

        EndDrawing();
    }
    CloseWindow();
    return 0;
}

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
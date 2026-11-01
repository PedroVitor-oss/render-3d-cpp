#include "face.h"

Vector3 Face::NormalFace(const std::vector<Vector3> &vertices) const
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

    float length = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
    if (length > 0.0001f)
    {
        normal.x /= length;
        normal.y /= length;
        normal.z /= length;
    }

    return normal;
}

bool Face::isFrontFacing(const std::vector<Vector3> &vertices, const enginep::Camera &cam) const
{
    Vector3 A = vertices[a];
    Vector3 B = vertices[b];
    Vector3 C = vertices[c];

    Vector3 normal = NormalFace(vertices);

    Vector3 toCam = {cam.camPos.x - A.x, cam.camPos.y - A.y, cam.camPos.z - A.z};

    float dot = normal.x * toCam.x + normal.y * toCam.y + normal.z * toCam.z;
    return dot > 0;
}

void Face::Submit(const std::vector<Vector3> &vertices, const enginep::Camera &cam,
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

void Face::draw(const std::vector<Vector3> &vertices, enginep::Camera &cam) const
{
    if (!isFrontFacing(vertices, cam))
        return;

    DrawTriangle(WorldToScreen(vertices[this->a], cam),
                 WorldToScreen(vertices[this->c], cam),
                 WorldToScreen(vertices[this->b], cam),
                 cl);
    this->drawWireframe(vertices, cam);
}

void Face::drawWireframe(const std::vector<Vector3> &vertices, enginep::Camera &cam) const
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
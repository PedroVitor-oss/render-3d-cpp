#include "math3d.h"
#include "camera.h"

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
        -sinf(rotate.y) * cosf(rotate.z) + cosf(rotate.y) * sinf(rotate.x) * sinf(rotate.z)};
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

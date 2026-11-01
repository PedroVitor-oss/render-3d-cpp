#include "renderqueue.h"

void RenderQueue::SetLight(GlobalLight *l)
{
    this->light = l;
}

void RenderQueue::Push(const ScreenTri &t) { tris.push_back(t); }

void RenderQueue::Flush()
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
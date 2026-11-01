#include "entity.h"

void Entity::SetScale(Vector3 s)
{

    this->scale = s;
}

void Entity::SetModel(Model3D m)
{
    this->model = m;
    this->saveModel = m;
}
void Entity::Render(enginep::Camera &cam, RenderQueue &queue)
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
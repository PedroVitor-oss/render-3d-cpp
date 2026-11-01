#ifndef MODEL3D_H
#define MODEL3D_H
#include "raylib.h"
#include "face.h"
#include "renderqueue.h"

#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <fstream>

class Model3D
{
private:
    std::vector<Face> faces;
    std::map<std::string, Color> textureMap;
    void AddVertice(std::string data);
    void AddFace(std::string data, Color color);
    void ReadModelOBJ(std::string path);
    void ReadTextureFile(std::string path);

public:
    std::vector<Vector3> vertices;
    void AddTriangle(int a, int b, int c, Color color);
    void BuildGrid(int tiles, float tileSize, Color c1, Color c2);
    void Init(std::string pathModels, std::string pathTexture = "");

    void Renderer(enginep::Camera &cam);
    void Submit(const enginep::Camera &cam, RenderQueue &queue, int layer);
};

#endif
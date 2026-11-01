#include "model3d.h"

void Model3D::AddVertice(std::string data)
{
    std::stringstream stream(data);

    char type;
    Vector3 v;
    stream >> type >> v.x >> v.y >> v.z;

    vertices.push_back(v);
}

void Model3D::AddFace(std::string data, Color color)
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

void Model3D::ReadModelOBJ(std::string path)
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
        if (prefix == "usemtl")
        {
            std::string colorName = line.substr(div + 1);
            if (textureMap.find(colorName) != textureMap.end())
            {
                currentColor = textureMap[colorName];
            }
        }
        else if (prefix == "v")
        {
            this->AddVertice(line);
        }
        else if (prefix == "f")
        {
            this->AddFace(line, currentColor);
        }
    }

    file.close();
}

void Model3D::ReadTextureFile(std::string path)
{
    std::fstream file;
    file.open(path);
    if (!file.is_open())
    {
        TraceLog(LOG_WARNING, "Erro ao abrir arquivo de textura, path: %s", path);
        file.close();
        return;
    }
    std::string line;
    std::string colorName;
    Color colorValue;
    bool seachColorValue = false;
    while (std::getline(file, line))
    {
        int div = line.find(" ");
        std::string prefix = line.substr(0, div);
        if (prefix == "newmtl")
        {
            colorName = line.substr(div + 1);
            seachColorValue = true;
        }
        if (prefix == "Kd" && seachColorValue)
        {
            std::stringstream stream(line.substr(div + 1));
            float r, g, b;
            stream >> r >> g >> b;
            colorValue.r = (unsigned char)(r * 255);
            colorValue.g = (unsigned char)(g * 255);
            colorValue.b = (unsigned char)(b * 255);
            colorValue.a = 255;
            textureMap[colorName] = colorValue;
            seachColorValue = false;
        }
    }
}

void Model3D::AddTriangle(int a, int b, int c, Color color)
{
    faces.push_back(Face(a, b, c, color));
}

void Model3D::BuildGrid(int tiles, float tileSize, Color c1, Color c2)
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

void Model3D::Init(std::string pathModels, std::string pathTexture )
{

    if (!pathTexture.empty())
    {
        this->ReadTextureFile(pathTexture);
    }
    this->ReadModelOBJ(pathModels);
}

void Model3D::Renderer(enginep::Camera &cam)
{
    for (const auto &f : this->faces)
        f.draw(vertices, cam);
}
void Model3D::Submit(const enginep::Camera &cam, RenderQueue &queue, int layer)
{
    for (const auto &f : this->faces)
        f.Submit(vertices, cam, queue, layer);
}
#include "ShapeBuilder.h"
#include <vector>
#include <cmath>

using namespace std;
using namespace m1;

Mesh* ShapeBuilder::CreateCylinder(const std::string& name, int segments)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;
    float height = 2.0f;
    float radius = 1.0f;

    vertices.push_back(VertexFormat(glm::vec3(0, height / 2, 0), glm::vec3(0, 1, 0)));
    vertices.push_back(VertexFormat(glm::vec3(0, -height / 2, 0), glm::vec3(0, -1, 0)));

    for (int i = 0; i < segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        vertices.push_back(VertexFormat(glm::vec3(x, height / 2, z), glm::vec3(x, 0, z)));
        vertices.push_back(VertexFormat(glm::vec3(x, -height / 2, z), glm::vec3(x, 0, z)));
    }

    for (int i = 0; i < segments; i++) {
        int topCenter = 0;
        int bottomCenter = 1;
        int topEdgeCurrent = 2 + i * 2;
        int bottomEdgeCurrent = 2 + i * 2 + 1;
        int topEdgeNext = 2 + ((i + 1) % segments) * 2;
        int bottomEdgeNext = 2 + ((i + 1) % segments) * 2 + 1;

        indices.push_back(topCenter); indices.push_back(topEdgeNext); indices.push_back(topEdgeCurrent);
        indices.push_back(bottomCenter); indices.push_back(bottomEdgeCurrent); indices.push_back(bottomEdgeNext);
        indices.push_back(bottomEdgeCurrent); indices.push_back(topEdgeCurrent); indices.push_back(topEdgeNext);
        indices.push_back(bottomEdgeCurrent); indices.push_back(topEdgeNext); indices.push_back(bottomEdgeNext);
    }

    Mesh* cylinder = new Mesh(name);
    cylinder->InitFromData(vertices, indices);
    return cylinder;
}

Mesh* ShapeBuilder::CreateBox(const std::string& name)
{
    std::vector<VertexFormat> vertices =
    {
        VertexFormat(glm::vec3(-0.5, -0.5,  0.5), glm::vec3(0, 0, 1)),
        VertexFormat(glm::vec3(0.5, -0.5,  0.5), glm::vec3(0, 0, 1)),
        VertexFormat(glm::vec3(0.5,  0.5,  0.5), glm::vec3(0, 0, 1)),
        VertexFormat(glm::vec3(-0.5,  0.5,  0.5), glm::vec3(0, 0, 1)),

        VertexFormat(glm::vec3(-0.5, -0.5, -0.5), glm::vec3(0, 0, -1)),
        VertexFormat(glm::vec3(0.5, -0.5, -0.5), glm::vec3(0, 0, -1)),
        VertexFormat(glm::vec3(0.5,  0.5, -0.5), glm::vec3(0, 0, -1)),
        VertexFormat(glm::vec3(-0.5,  0.5, -0.5), glm::vec3(0, 0, -1)),
    };

    std::vector<unsigned int> indices =
    {
        0, 1, 2,    0, 2, 3,  // Fata
        1, 5, 6,    1, 6, 2,  // Dreapta
        5, 4, 7,    5, 7, 6,  // Spate
        4, 0, 3,    4, 3, 7,  // Stanga
        3, 2, 6,    3, 6, 7,  // Sus
        4, 5, 1,    4, 1, 0   // Jos
    };

    Mesh* box = new Mesh(name);
    box->InitFromData(vertices, indices);
    return box;
}

Mesh* ShapeBuilder::CreateSquare(const std::string& name, float length)
{
    glm::vec3 corner = glm::vec3(0, 0, 0); // Coltul stanga-jos
    length = length / 2.0f; // Ca sa fie centrat

    std::vector<VertexFormat> vertices =
    {
        VertexFormat(glm::vec3(-length, 0,  length), glm::vec3(0, 1, 0)), // 0
        VertexFormat(glm::vec3(length, 0,  length), glm::vec3(0, 1, 0)), // 1
        VertexFormat(glm::vec3(length, 0, -length), glm::vec3(0, 1, 0)), // 2
        VertexFormat(glm::vec3(-length, 0, -length), glm::vec3(0, 1, 0))  // 3
    };

    std::vector<unsigned int> indices =
    {
        0, 1, 2,
        0, 2, 3
    };

    Mesh* square = new Mesh(name);
    square->InitFromData(vertices, indices);
    return square;
}

Mesh* ShapeBuilder::CreateCone(const std::string& name, float height, float radius, int slices)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Varful conului
    vertices.emplace_back(glm::vec3(0, height, 0), glm::vec3(0, 1, 0), glm::vec3(1));

    // Centrul bazei
    vertices.emplace_back(glm::vec3(0, 0, 0), glm::vec3(0, -1, 0), glm::vec3(1));

    // Punctele de pe cerc
    for (int i = 0; i < slices; i++)
    {
        float angle = 2.0f * 3.14159f * i / slices;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        vertices.emplace_back(glm::vec3(x, 0, z), glm::vec3(x, 0, z), glm::vec3(1));
    }

    // Indicii
    int centerIdx = 1;
    int startIdx = 2;

    for (int i = 0; i < slices; i++)
    {
        int next = (i + 1) % slices;

        // Triunghiuri laterale (Varf -> Cerc -> Cerc_Next)
        indices.push_back(0);
        indices.push_back(startIdx + i);
        indices.push_back(startIdx + next);

        // Triunghiuri baza (Centru -> Cerc_Next -> Cerc)
        indices.push_back(centerIdx);
        indices.push_back(startIdx + next);
        indices.push_back(startIdx + i);
    }

    Mesh* mesh = new Mesh(name);
    mesh->InitFromData(vertices, indices);
    return mesh;
}

Mesh* ShapeBuilder::CreateArrow(const std::string& name)
{

    std::vector<VertexFormat> vertices = {
        // Varful (Indreapta spre -Z)
        VertexFormat(glm::vec3(0.0f, 0.0f, -2.0f), glm::vec3(0, 1, 0)),  // 0

        // Spate Stanga
        VertexFormat(glm::vec3(-1.0f, 0.0f, 1.0f), glm::vec3(0, 1, 0)),  // 1

        // Spate Centru (Scobitura interioara)
        VertexFormat(glm::vec3(0.0f, 0.0f, 0.5f), glm::vec3(0, 1, 0)),   // 2

        // Spate Dreapta
        VertexFormat(glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0, 1, 0))    // 3
    };

    std::vector<unsigned int> indices = {
        0, 1, 2,
        0, 2, 3
    };

    Mesh* arrow = new Mesh(name);
    arrow->InitFromData(vertices, indices);
    return arrow;
}
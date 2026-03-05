#include "Environment.h"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <GLFW/glfw3.h> 

using namespace std;
using namespace m1;

Environment::Environment()
{
    // Initializare harta default cu iarba
    for (int i = 0; i < GRID_SIZE; i++)
        for (int j = 0; j < GRID_SIZE; j++)
            map[i][j] = GRASS;
}

Environment::~Environment() {}

void Environment::SetRegion(int startX, int startZ, int endX, int endZ, TerrainType type)
{
    int mapStartX = std::max(0, std::min(GRID_SIZE - 1, startX + OFFSET));
    int mapStartZ = std::max(0, std::min(GRID_SIZE - 1, startZ + OFFSET));
    int mapEndX = std::max(0, std::min(GRID_SIZE - 1, endX + OFFSET));
    int mapEndZ = std::max(0, std::min(GRID_SIZE - 1, endZ + OFFSET));

    for (int x = mapStartX; x <= mapEndX; x++) {
        for (int z = mapStartZ; z <= mapEndZ; z++) {
            map[x][z] = type;
        }
    }
}

void Environment::Init()
{
    // Harta ASCII pentru generarea terenului 20 x 20
    std::vector<std::string> layout = {
        "MMMMMMMMMMMMMMGGGGGG",
        "MMMMMMMMMMMGGGGGGGGG",
        "GGGGGGGGMMMWWWGGGGGG",
        "GGGGGGGGMMMWWWGGGGGG",
        "GGGGGGGGGGGGWWGGGGGG",
        "GGGGGGGGGGGGGGGGGGGG",
        "MMMMMGGGGGGGGGGGWWWG",
        "MMMMMGGGGGGGGGGGWWWG",
        "MMMMMMMGGGGGGGGGWWWG",
        "MMMMMMMGGGGGGGGGGGGG",
        "GGGMMGGGGGGGGGGGGGGG",
        "GGGGGGGGGGGGGGGGGGGG",
        "GGGGGGGGGGGGGGGGGMMM",
        "GGGGGWWGGGGGGGGGGMMM",
        "GGGGWWWWGGGGGGGGMMMM",
        "GGGGGGGGGGGWWWWGGGMM",
        "GGGGGGGGGGGGWWWGGGMM",
        "GGMMGGGGGGGGGGGGGGGG",
        "GGMMMGGGGGGGGGGGGGGG",
        "GGMMMGGGGGGGGGGGGGGG"
    };

    int rows = layout.size();
    int cols = layout[0].size();
    float stepX = (float)GRID_SIZE / cols;
    float stepZ = (float)GRID_SIZE / rows;

    // Parsare layout si populare matrice map
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            char c = layout[i][j];
            TerrainType type = GRASS;

            if (c == 'W') type = WATER;
            else if (c == 'M') type = MOUNTAIN;

            int startX = (int)(j * stepX) - OFFSET;
            int startZ = (int)(i * stepZ) - OFFSET;
            int endX = (int)((j + 1) * stepX) - OFFSET;
            int endZ = (int)((i + 1) * stepZ) - OFFSET;

            SetRegion(startX, startZ, endX - 1, endZ - 1, type);
        }
    }
}

void Environment::RenderMeshColor(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color, tema::Camera* camera, const glm::mat4& projectionMatrix, bool isBroken)
{
    if (!mesh || !shader || !shader->program) return;

    shader->Use();

    glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
    glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    GLint locObjectColor = glGetUniformLocation(shader->program, "object_color");
    if (locObjectColor != -1) glUniform3f(locObjectColor, color.x, color.y, color.z);

    float currentTime = (float)glfwGetTime();
    GLint locTime = glGetUniformLocation(shader->program, "time");
    if (locTime != -1) glUniform1f(locTime, currentTime);

    GLint locBroken = glGetUniformLocation(shader->program, "is_broken");
    if (locBroken != -1) glUniform1i(locBroken, isBroken ? 1 : 0);

    mesh->Render();
}

void Environment::Render(RailManager* railManager, tema::Camera* camera, const glm::mat4& projectionMatrix, const std::unordered_map<std::string, Mesh*>& meshes, const std::unordered_map<std::string, Shader*>& shaders)
{
    if (!railManager) return;
    if (meshes.find("box") == meshes.end() || shaders.find("Simple") == shaders.end()) return;

    Mesh* boxMesh = meshes.at("box");
    Shader* shader = shaders.at("Simple");
    const std::vector<Rail*>& rails = railManager->GetRails();

	// Randare teren
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int z = 0; z < GRID_SIZE; z++) {
            int type = map[x][z];
            float worldX = (float)(x - OFFSET);
            float worldZ = (float)(z - OFFSET);

            glm::vec3 pos, scale, color;

            if (type == WATER) {
                pos = glm::vec3(worldX, 0.0f, worldZ);
                scale = glm::vec3(1.0f, 0.1f, 1.0f);
                color = glm::vec3(0.2f, 0.4f, 0.9f);
            }
            else if (type == GRASS) {
                pos = glm::vec3(worldX, 0.0f, worldZ);
                scale = glm::vec3(1.0f, 0.2f, 1.0f);
                color = glm::vec3(0.3f, 0.8f, 0.3f);
            }
            else {
                pos = glm::vec3(worldX, 0.0f, worldZ);
                scale = glm::vec3(1.0f, 0.2f, 1.0f);
                color = glm::vec3(0.4f, 0.25f, 0.15f);
            }

            glm::mat4 model = glm::mat4(1);
            model = glm::translate(model, pos);
            model = glm::scale(model, scale);

            RenderMeshColor(boxMesh, shader, model, color, camera, projectionMatrix, false);
        }
    }

    // Randare sine
    static float timeAccumulator = 0.0f;
    timeAccumulator += 0.05f;

    for (const auto& rail : rails) {
        glm::vec3 start = rail->startPosition;
        glm::vec3 end = rail->endPosition;
        glm::vec3 center = (start + end) / 2.0f;
        float length = glm::distance(start, end);

        // Calcul orientare sina
        glm::vec3 direction = glm::normalize(end - start);
        glm::vec3 axis = glm::cross(glm::vec3(1, 0, 0), direction);
        float angle = acos(glm::dot(glm::vec3(1, 0, 0), direction));

        glm::mat4 baseTransform = glm::mat4(1);
        baseTransform = glm::translate(baseTransform, center);

        if (glm::length(axis) > 0.001f)
            baseTransform = glm::rotate(baseTransform, angle, axis);
        else if (direction.x < 0)
            baseTransform = glm::rotate(baseTransform, RADIANS(180), glm::vec3(0, 1, 0));

        // Efect vizual pentru sina stricata
        bool isBroken = rail->isBroken;
        glm::vec3 railColor = glm::vec3(0.1f, 0.1f, 0.1f);

        if (isBroken) {
            float flicker = (sin(timeAccumulator) + 1.0f) * 0.5f;
            railColor = glm::vec3(0.8f + flicker * 0.2f, 0.0f, 0.0f);
            baseTransform = glm::rotate(baseTransform, RADIANS(10), glm::vec3(0, 1, 0)); // Rotatie usoara sa para deraiata
        }

        // Randare in functie de tipul sinei
        if (rail->type == "Surface") {
            int numSleepers = (int)(length * 1.5f);
            for (int i = 0; i < numSleepers; i++) {
                float xOff = -length / 2.0f + (length / numSleepers) * i;

                glm::mat4 sleeper = glm::translate(baseTransform, glm::vec3(xOff, 0.0f, 0));
                sleeper = glm::scale(sleeper, glm::vec3(0.3f, 0.05f, 1.2f));

                glm::vec3 sleeperColor = isBroken ? glm::vec3(0.5f, 0.2f, 0.2f) : glm::vec3(0.4f, 0.2f, 0.1f);
                RenderMeshColor(boxMesh, shader, sleeper, sleeperColor, camera, projectionMatrix, isBroken);
            }
        }
        else if (rail->type == "Bridge") {
            int numPillars = (int)(length / 6.0f) + 1;
            for (int i = 0; i < numPillars; i++) {
                float xPos = -length / 2.0f + (length / (numPillars - 1)) * i;

                glm::mat4 pillar = glm::translate(baseTransform, glm::vec3(xPos, -0.5f, 0));
                pillar = glm::scale(pillar, glm::vec3(1.0f, 1.0f, 1.0f));

                RenderMeshColor(boxMesh, shader, pillar, glm::vec3(0.5f, 0.5f, 0.5f), camera, projectionMatrix, false);
            }

            glm::mat4 deck = glm::translate(baseTransform, glm::vec3(0, -0.1f, 0));
            deck = glm::scale(deck, glm::vec3(length - 0.2f, 0.2f, 1.5f));

            glm::vec3 deckColor = isBroken ? glm::vec3(0.6f, 0.3f, 0.3f) : glm::vec3(0.6f, 0.6f, 0.6f);
            RenderMeshColor(boxMesh, shader, deck, deckColor, camera, projectionMatrix, isBroken);
        }
        else if (rail->type == "Tunnel") {
            float tunnelOffset = 0.5f;
            glm::vec3 wallColor = glm::vec3(0.3f, 0.2f, 0.1f);

            // Pereti laterali
            glm::mat4 wallL = glm::translate(baseTransform, glm::vec3(tunnelOffset, 1.25f, 1.5f));
            wallL = glm::scale(wallL, glm::vec3(length, 2.5f, 0.5f));
            RenderMeshColor(boxMesh, shader, wallL, wallColor, camera, projectionMatrix, false);

            glm::mat4 wallR = glm::translate(baseTransform, glm::vec3(tunnelOffset, 1.25f, -1.5f));
            wallR = glm::scale(wallR, glm::vec3(length, 2.5f, 0.5f));
            RenderMeshColor(boxMesh, shader, wallR, wallColor, camera, projectionMatrix, false);

            // Acoperis
            glm::mat4 roof = glm::translate(baseTransform, glm::vec3(tunnelOffset, 2.75f, 0));
            roof = glm::scale(roof, glm::vec3(length, 0.5f, 3.5f));
            RenderMeshColor(boxMesh, shader, roof, wallColor, camera, projectionMatrix, false);
        }

        float railSpacing = 0.4f;

        glm::mat4 railL = glm::translate(baseTransform, glm::vec3(0, 0.05f, railSpacing));
        railL = glm::scale(railL, glm::vec3(length, 0.05f, 0.1f));
        RenderMeshColor(boxMesh, shader, railL, railColor, camera, projectionMatrix, isBroken);

        glm::mat4 railR = glm::translate(baseTransform, glm::vec3(0, 0.05f, -railSpacing));
        railR = glm::scale(railR, glm::vec3(length, 0.05f, 0.1f));
        RenderMeshColor(boxMesh, shader, railR, railColor, camera, projectionMatrix, isBroken);
    }

	// Randare statii
    const std::vector<Station>& stations = railManager->GetStations();
    for (const auto& station : stations) {
        if (meshes.find(station.meshType) != meshes.end() && shaders.find("Simple") != shaders.end()) {

            glm::mat4 modelMatrix = glm::mat4(1);
            float yOffset = (station.meshType == "cone") ? 0.0f : 1.0f;

            modelMatrix = glm::translate(modelMatrix, station.position + glm::vec3(0, yOffset, 0));

            // Scalare in functie de tip
            if (station.meshType == "box") modelMatrix = glm::scale(modelMatrix, glm::vec3(3.0f, 2.0f, 2.0f));
            else if (station.meshType == "cone") modelMatrix = glm::scale(modelMatrix, glm::vec3(1.5f, 1.5f, 1.5f));
            else modelMatrix = glm::scale(modelMatrix, glm::vec3(1.5f, 1.0f, 1.5f));

            // Culoare in functie de tip
            glm::vec3 stationColor;
            if (station.meshType == "box") stationColor = glm::vec3(0.8f, 0.4f, 0.4f);
            else if (station.meshType == "cone") stationColor = glm::vec3(0.9f, 0.8f, 0.2f);
            else stationColor = glm::vec3(0.3f, 0.6f, 0.9f);

            RenderMeshColor(meshes.at(station.meshType), shaders.at("Simple"), modelMatrix, stationColor, camera, projectionMatrix, false);
        }
    }
}
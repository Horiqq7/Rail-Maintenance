#pragma once

#include "components/simple_scene.h"
#include "RailManager.h"
#include "Tema2_Camera.h" 

namespace m1
{
    enum TerrainType {
        WATER = 0,
        GRASS = 1,
        MOUNTAIN = 2
    };

    class Environment
    {
    public:
        Environment();
        ~Environment();

        // Initializare layout harta (apa, munti, iarba)
        void Init();

        // Functia principala de randare a intregii scene
        void Render(RailManager* railManager, tema::Camera* camera, const glm::mat4& projectionMatrix, 
            const std::unordered_map<std::string, Mesh*>& meshes, const std::unordered_map<std::string, Shader*>& shaders);

    private:
        void RenderMeshColor(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color, 
            tema::Camera* camera, const glm::mat4& projectionMatrix, bool isBroken);
        void SetRegion(int startX, int startZ, int endX, int endZ, TerrainType type);

    private:
        static const int GRID_SIZE = 120;
        static const int OFFSET = 60;
        int map[GRID_SIZE][GRID_SIZE];
    };
}
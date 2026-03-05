#pragma once

#include "components/simple_scene.h"
#include "components/text_renderer.h"
#include "RailManager.h"
#include "Train.h"
#include "Tema2_Camera.h"
#include "Environment.h"

namespace m1
{
    class Tema2 : public gfxc::SimpleScene
    {
    public:
        Tema2();
        ~Tema2();

        void Init() override;

    private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        void RenderMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix) override;

        void RenderTrains();
        void RenderLocomotive(const glm::mat4& modelMatrix, glm::vec3 colorBody, glm::vec3 colorCabin, glm::vec3 colorEngine);
        void RenderWagon(const glm::mat4& modelMatrix, glm::vec3 colorBody);
        void RenderDrezina(const glm::vec3& position, float rotationAngle, float handleAngle);

        void RenderMinimap(const std::vector<Rail*>& rails, const std::vector<Train*>& trains, const glm::vec3& playerPos,
            float playerRotation, Mesh* boxMesh, Mesh* coneMesh, Mesh* cylinderMesh, Shader* shader, glm::ivec2 resolution);

    protected:
        // Camera
        tema::Camera* camera;
        glm::mat4 projectionMatrix;
        bool renderCameraTarget;

        bool is_orthographic;
        float fov;
        float leftOrtho, rightOrtho, bottomOrtho, topOrtho;

        // Componente
        RailManager* railManager;
        Environment* environment;
        std::vector<Train*> trains;
        gfxc::TextRenderer* textRenderer;

        // Drezina
        bool controlDrezina;
        glm::vec3 drezinaPosition;
        float drezinaRotation;

        // Logica joc
        bool isRepairing;
        float repairTimer;
        Rail* railToRepair;

        float breakTimer;
        float totalGameTime;

        bool isGamePaused;
        bool isGameOver;
    };
}